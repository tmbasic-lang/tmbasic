#include "compiler/compileTypes.h"
#include "compiler/ast.h"
#include "compiler/bindNamedRecordTypes.h"
#include "compiler/checkRecursiveTypes.h"
#include "compiler/CompilerException.h"
#include "compiler/parse.h"
#include "compiler/tokenize.h"
#include "shared/cast.h"
#include "shared/strings.h"

namespace compiler {

static void compileType(int sourceMemberIndex, const SourceMember& sourceMember, CompiledProgram* compiledProgram) {
    auto tokens = tokenize(sourceMember.source + "\n", TokenizeType::kCompile, &sourceMember);
    auto parserResult = parse(&sourceMember, ParserRootProduction::kMember, tokens);
    if (!parserResult.isSuccess) {
        throw CompilerException(CompilerErrorCode::kSyntax, parserResult.message, *parserResult.token);
    }

    if (parserResult.node->getMemberType() != MemberType::kTypeDeclaration) {
        throw CompilerException(
            CompilerErrorCode::kWrongMemberType, "This member was expected to be a type declaration.", tokens[0]);
    }

    const auto& typeDeclarationNode = dynamic_cast<TypeDeclarationNode&>(*parserResult.node);
    auto lowercaseIdentifier = shared::to_lower_copy(typeDeclarationNode.name);

    if (compiledProgram->userTypesByNameLowercase.find(lowercaseIdentifier) !=
        compiledProgram->userTypesByNameLowercase.end()) {
        throw CompilerException(
            CompilerErrorCode::kDuplicateTypeName,
            fmt::format("A type named \"{}\" already exists.", typeDeclarationNode.name), {});
    }

    auto t = std::make_unique<CompiledUserType>();
    t->nameLowercase = lowercaseIdentifier;
    t->name = typeDeclarationNode.name;

    auto nextValueIndex = 0;
    auto nextObjectIndex = 0;
    for (const auto& field : typeDeclarationNode.fields) {
        auto compiledField = std::make_unique<CompiledUserTypeField>();
        compiledField->nameLowercase = shared::to_lower_copy(field->name);
        compiledField->name = field->name;
        compiledField->isValue = field->type->isValueType();
        compiledField->isObject = !compiledField->isValue;
        compiledField->fieldIndex = compiledField->isValue ? nextValueIndex++ : nextObjectIndex++;
        compiledField->parameterNode = boost::make_local_shared<ParameterNode>(field->name, field->type, field->token);
        if (compiledField->isValue) {
            compiledField->parameterNode->fieldValueIndex = compiledField->fieldIndex;
        } else {
            compiledField->parameterNode->fieldObjectIndex = compiledField->fieldIndex;
        }
        t->fieldsByNameLowercase.insert(std::make_pair(compiledField->nameLowercase, compiledField.get()));
        t->fields.push_back(std::move(compiledField));
    }

    compiledProgram->userTypesByNameLowercase.insert(std::make_pair(lowercaseIdentifier, t.get()));
    compiledProgram->userTypesBySourceMemberIndex.insert(std::make_pair(sourceMemberIndex, t.get()));
    compiledProgram->userTypes.push_back(std::move(t));
}

static void checkFieldType(const TypeNode& fieldTypeNode, const CompiledProgram& compiledProgram) {
    if (fieldTypeNode.kind == Kind::kRecord && fieldTypeNode.recordName.has_value()) {
        auto lowercaseRecordName = shared::to_lower_copy(*fieldTypeNode.recordName);
        if (compiledProgram.userTypesByNameLowercase.find(lowercaseRecordName) ==
            compiledProgram.userTypesByNameLowercase.end()) {
            throw CompilerException(
                CompilerErrorCode::kTypeNotFound,
                fmt::format("The type \"{}\" is not defined.", *fieldTypeNode.recordName), fieldTypeNode.token);
        }
    } else if (fieldTypeNode.kind == Kind::kList) {
        checkFieldType(*fieldTypeNode.listItemType, compiledProgram);
    } else if (fieldTypeNode.kind == Kind::kMap) {
        checkFieldType(*fieldTypeNode.mapKeyType, compiledProgram);
        checkFieldType(*fieldTypeNode.mapValueType, compiledProgram);
    } else if (fieldTypeNode.kind == Kind::kSet) {
        checkFieldType(*fieldTypeNode.setKeyType, compiledProgram);
    } else if (fieldTypeNode.kind == Kind::kOptional) {
        checkFieldType(*fieldTypeNode.optionalValueType, compiledProgram);
    }
}

static void checkFieldTypes(size_t sourceMemberIndex, CompiledProgram* compiledProgram) {
    auto* compiledUserType = compiledProgram->userTypesBySourceMemberIndex.find(sourceMemberIndex)->second;
    for (auto& field : compiledUserType->fields) {
        checkFieldType(*field->parameterNode->type, *compiledProgram);
    }
}

static void bindFieldTypes(size_t sourceMemberIndex, CompiledProgram* compiledProgram) {
    auto* compiledUserType = compiledProgram->userTypesBySourceMemberIndex.find(sourceMemberIndex)->second;
    for (auto& field : compiledUserType->fields) {
        bindNamedRecordTypes(field->parameterNode->type.get(), *compiledProgram);
    }
}

void compileTypes(const SourceProgram& sourceProgram, CompiledProgram* compiledProgram) {
    // Build a CompiledUserType for each user type, which assigns field indices.
    sourceProgram.forEachMemberIndex(SourceMemberType::kType, [compiledProgram](const auto& sourceMember, auto index) {
        compileType(static_cast<int>(index), sourceMember, compiledProgram);
    });

    // Check that fields of record types refer to defined types.
    sourceProgram.forEachMemberIndex(
        SourceMemberType::kType,
        [compiledProgram](const auto& /*sourceMember*/, auto index) { checkFieldTypes(index, compiledProgram); });

    // We must check for cycles AFTER building CompiledUserTypes but BEFORE binding named record types.
    // If there's a cycle and we try to bind it, we'll create an internal cycle in the reference counted TypeNodes
    // which will cause a memory leak.
    checkRecursiveTypes(sourceProgram, compiledProgram);

    // Binds the named record types to their declarations.
    sourceProgram.forEachMemberIndex(
        SourceMemberType::kType,
        [compiledProgram](const auto& /*sourceMember*/, auto index) { bindFieldTypes(index, compiledProgram); });
}

}  // namespace compiler
