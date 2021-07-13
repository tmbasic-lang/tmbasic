#include "compileTypes.h"
#include "CompilerException.h"
#include "ast.h"
#include "parse.h"
#include "tokenize.h"
#include "util/cast.h"

namespace compiler {

static void compileType(int sourceMemberIndex, const SourceMember& sourceMember, CompiledProgram* compiledProgram) {
    auto tokens = tokenize(sourceMember.source + "\n", TokenizeType::kCompile, &sourceMember);
    auto parserResult = parse(&sourceMember, ParserRootProduction::kMember, tokens);
    if (!parserResult.isSuccess) {
        throw CompilerException(parserResult.message, *parserResult.token);
    }

    if (parserResult.node->getMemberType() != MemberType::kTypeDeclaration) {
        throw CompilerException("This member was expected to be a type declaration.", tokens[0]);
    }

    const auto& typeDeclarationNode = dynamic_cast<TypeDeclarationNode&>(*parserResult.node);
    auto lowercaseIdentifier = boost::to_lower_copy(typeDeclarationNode.name);

    if (compiledProgram->userTypesByNameLowercase.find(lowercaseIdentifier) !=
        compiledProgram->userTypesByNameLowercase.end()) {
        throw CompilerException(fmt::format("A type named \"{}\" already exists.", typeDeclarationNode.name), {});
    }

    auto t = std::make_unique<CompiledUserType>();
    t->nameLowercase = lowercaseIdentifier;
    t->name = typeDeclarationNode.name;

    auto nextValueIndex = 0;
    auto nextObjectIndex = 0;
    for (const auto& field : typeDeclarationNode.fields) {
        auto compiledField = std::make_unique<CompiledUserTypeField>();
        compiledField->nameLowercase = boost::to_lower_copy(field->name);
        compiledField->name = field->name;
        compiledField->isValue = field->type->isValueType();
        compiledField->isObject = !compiledField->isValue;
        compiledField->fieldIndex = compiledField->isValue ? nextValueIndex++ : nextObjectIndex++;
        compiledField->type = std::move(field->type);
        t->fieldsByNameLowercase.insert(std::make_pair(compiledField->nameLowercase, compiledField.get()));
        t->fields.push_back(std::move(compiledField));
    }

    compiledProgram->userTypesByNameLowercase.insert(std::make_pair(lowercaseIdentifier, t.get()));
    compiledProgram->userTypesBySourceMemberIndex.insert(std::make_pair(sourceMemberIndex, t.get()));
    compiledProgram->userTypes.push_back(std::move(t));
}

static void checkFieldType(const TypeNode& fieldTypeNode, const CompiledProgram& compiledProgram) {
    if (fieldTypeNode.kind == Kind::kRecord && fieldTypeNode.recordName.has_value()) {
        auto lowercaseRecordName = boost::to_lower_copy(*fieldTypeNode.recordName);
        if (compiledProgram.userTypesByNameLowercase.find(lowercaseRecordName) ==
            compiledProgram.userTypesByNameLowercase.end()) {
            throw CompilerException(
                fmt::format("The type \"{}\" is not defined.", *fieldTypeNode.recordName), fieldTypeNode.token);
        }
    } else if (fieldTypeNode.kind == Kind::kList) {
        checkFieldType(*fieldTypeNode.listItemType, compiledProgram);
    } else if (fieldTypeNode.kind == Kind::kMap) {
        checkFieldType(*fieldTypeNode.mapKeyType, compiledProgram);
        checkFieldType(*fieldTypeNode.mapValueType, compiledProgram);
    }
}

static void checkFieldTypes(size_t sourceMemberIndex, CompiledProgram* compiledProgram) {
    auto* compiledUserType = compiledProgram->userTypesBySourceMemberIndex.find(sourceMemberIndex)->second;
    for (auto& field : compiledUserType->fields) {
        checkFieldType(*field->type, *compiledProgram);
    }
}

void compileTypes(const SourceProgram& sourceProgram, CompiledProgram* compiledProgram) {
    // build a CompiledUserType for each user type, which assigns field indices
    sourceProgram.forEachMemberIndex(SourceMemberType::kType, [compiledProgram](const auto& sourceMember, auto index) {
        compileType(index, sourceMember, compiledProgram);
    });

    // check that fields of record types refer to defined types
    sourceProgram.forEachMemberIndex(
        SourceMemberType::kType,
        [compiledProgram](const auto& /*sourceMember*/, auto index) { checkFieldTypes(index, compiledProgram); });
}

}  // namespace compiler
