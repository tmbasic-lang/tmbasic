#include "compileTypes.h"
#include "compiler/ast.h"
#include "compiler/parse.h"
#include "compiler/tokenize.h"
#include "util/cast.h"

namespace compiler {

static CompilerResult compileType(const SourceMember& sourceMember, CompiledProgram* compiledProgram) {
    auto tokens = tokenize(sourceMember.source + "\n", TokenizeType::kCompile, &sourceMember);
    auto parserResult = parse(&sourceMember, ParserRootProduction::kMember, tokens);
    if (!parserResult.isSuccess) {
        return CompilerResult::error(parserResult.message, *parserResult.token);
    }

    if (parserResult.node->getMemberType() != MemberType::kTypeDeclaration) {
        return CompilerResult::error("This member was expected to be a type declaration.", tokens[0]);
    }

    const auto& typeDeclarationNode = dynamic_cast<TypeDeclarationNode&>(*parserResult.node);
    auto lowercaseIdentifier = boost::algorithm::to_lower_copy(typeDeclarationNode.name);

    if (compiledProgram->userTypesByNameLowercase.find(lowercaseIdentifier) !=
        compiledProgram->userTypesByNameLowercase.end()) {
        return CompilerResult::error(fmt::format("A type named \"{}\" already exists.", typeDeclarationNode.name), {});
    }

    auto t = std::make_unique<CompiledUserType>();
    t->nameLowercase = lowercaseIdentifier;
    t->name = typeDeclarationNode.name;

    auto nextValueIndex = 0;
    auto nextObjectIndex = 0;
    for (const auto& field : typeDeclarationNode.fields) {
        auto compiledField = std::make_unique<CompiledUserTypeField>();
        compiledField->nameLowercase = boost::algorithm::to_lower_copy(field->name);
        compiledField->name = field->name;
        compiledField->isValue = field->type->isValueType();
        compiledField->isObject = !compiledField->isValue;
        compiledField->fieldIndex = compiledField->isValue ? nextValueIndex++ : nextObjectIndex++;
        t->fieldsByNameLowercase.insert(std::make_pair(compiledField->nameLowercase, compiledField.get()));
        t->fields.push_back(std::move(compiledField));
    }

    compiledProgram->userTypesByNameLowercase.insert(std::make_pair(lowercaseIdentifier, t.get()));
    compiledProgram->userTypes.push_back(std::move(t));

    return CompilerResult::success();
}

CompilerResult compileTypes(const SourceProgram& sourceProgram, CompiledProgram* compiledProgram) {
    for (const auto& member : sourceProgram.members) {
        if (member->memberType == SourceMemberType::kType) {
            auto result = compileType(*member, compiledProgram);
            if (!result.isSuccess) {
                return result;
            }
        }
    }

    return CompilerResult::success();
}

}  // namespace compiler
