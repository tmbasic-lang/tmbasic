#include "compiler/bindNamedTypes.h"

using basic::BodyNode;
using basic::ProcedureNode;
using basic::TypeNode;

namespace compiler {

static CompilerResult bindNamedTypesInType(TypeNode* typeNode, const vm::Program& program) {
    if (typeNode->recordName.has_value()) {
        // auto& name = *typeNode.recordName;
        // TODO: find type
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

    return CompilerResult::success();
}

static CompilerResult bindNamedTypesInBody(basic::BodyNode* body, const vm::Program& program);

static CompilerResult bindNamedTypesInStatement(basic::StatementNode* node, const vm::Program& program) {
    auto result = CompilerResult::success();

    auto childTypeNode = node->getChildTypeNode();
    if (childTypeNode) {
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

CompilerResult bindNamedTypesInBody(BodyNode* body, const vm::Program& program) {
    for (auto& statement : body->statements) {
        auto result = bindNamedTypesInStatement(statement.get(), program);
        if (!result.isSuccess) {
            return result;
        }
    }
    return CompilerResult::success();
}

CompilerResult bindNamedTypes(ProcedureNode* procedure, const vm::Program& program) {
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
