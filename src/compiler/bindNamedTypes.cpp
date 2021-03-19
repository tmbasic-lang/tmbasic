#include "compiler/bindNamedTypes.h"

namespace compiler {

static CompilerResult bindNamedTypesInType(TypeNode* typeNode, const CompiledProgram& program) {
    if (typeNode->namedTypesAreBound) {
        return CompilerResult::success();
    }

    if (typeNode->recordName.has_value()) {
        const auto name = boost::algorithm::to_lower_copy(*typeNode->recordName);
        for (const auto& compiledNamedType : program.namedTypes) {
            if (name == compiledNamedType->lowercaseName) {
                typeNode->fields = compiledNamedType->fields;
            }
        }
    }

    for (auto& field : typeNode->fields) {
        auto result = bindNamedTypesInType(field->type.get(), program);
        if (!result.isSuccess) {
            return result;
        }
    }

    if (typeNode->listItemType) {
        auto result = bindNamedTypesInType(typeNode->listItemType.get(), program);
        if (!result.isSuccess) {
            return result;
        }
    }

    if (typeNode->mapKeyType) {
        auto result = bindNamedTypesInType(typeNode->mapKeyType.get(), program);
        if (!result.isSuccess) {
            return result;
        }
    }

    if (typeNode->mapValueType) {
        auto result = bindNamedTypesInType(typeNode->mapValueType.get(), program);
        if (!result.isSuccess) {
            return result;
        }
    }

    if (typeNode->optionalValueType) {
        auto result = bindNamedTypesInType(typeNode->optionalValueType.get(), program);
        if (!result.isSuccess) {
            return result;
        }
    }

    typeNode->namedTypesAreBound = true;

    return CompilerResult::success();
}

static CompilerResult bindNamedTypesInBody(BodyNode* body, const CompiledProgram& program);

static CompilerResult bindNamedTypesInStatement(StatementNode* node, const CompiledProgram& program) {
    auto result = CompilerResult::success();

    auto* childTypeNode = node->getChildTypeNode();
    if (childTypeNode != nullptr) {
        result = bindNamedTypesInType(childTypeNode, program);
        if (!result.isSuccess) {
            return result;
        }
    }

    node->visitBodies([&program, &result](BodyNode& body) -> bool {
        result = bindNamedTypesInBody(&body, program);
        return result.isSuccess;
    });

    return result;
}

CompilerResult bindNamedTypesInBody(BodyNode* body, const CompiledProgram& program) {
    for (auto& statement : body->statements) {
        auto result = bindNamedTypesInStatement(statement.get(), program);
        if (!result.isSuccess) {
            return result;
        }
    }
    return CompilerResult::success();
}

CompilerResult bindNamedTypes(ProcedureNode* procedure, const CompiledProgram& program) {
    for (auto& parameter : procedure->parameters) {
        auto result = bindNamedTypesInType(parameter->type.get(), program);
        if (!result.isSuccess) {
            return result;
        }
    }

    if (procedure->returnType) {
        auto result = bindNamedTypesInType(procedure->returnType.get(), program);
        if (!result.isSuccess) {
            return result;
        }
    }

    auto bodyResult = bindNamedTypesInBody(procedure->body.get(), program);
    if (!bodyResult.isSuccess) {
        return bodyResult;
    }

    return CompilerResult::success();
}

}  // namespace compiler
