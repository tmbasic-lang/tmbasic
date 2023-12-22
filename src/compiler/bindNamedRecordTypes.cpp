#include "bindNamedRecordTypes.h"
#include "CompilerException.h"
#include "findBuiltInRecordType.h"

namespace compiler {

class NamedRecordTypeBinder {
   private:
    const CompiledProgram& _compiledProgram;

   public:
    explicit NamedRecordTypeBinder(const CompiledProgram& compiledProgram) : _compiledProgram(compiledProgram) {}

    void bindType(TypeNode* typeNode) {
        for (auto& parameterNode : typeNode->fields) {
            bindParameter(parameterNode.get());
        }

        if (typeNode->listItemType != nullptr) {
            bindType(typeNode->listItemType.get());
        }

        if (typeNode->mapKeyType != nullptr) {
            bindType(typeNode->mapKeyType.get());
        }

        if (typeNode->mapValueType != nullptr) {
            bindType(typeNode->mapValueType.get());
        }

        if (typeNode->setKeyType != nullptr) {
            bindType(typeNode->setKeyType.get());
        }

        if (typeNode->optionalValueType != nullptr) {
            bindType(typeNode->optionalValueType.get());
        }

        if (typeNode->recordName.has_value()) {
            auto recordNameLowercase = boost::to_lower_copy(*typeNode->recordName);

            if (findBuiltInRecordType(recordNameLowercase, &typeNode->fields)) {
                return;
            }

            const auto& userType = _compiledProgram.userTypesByNameLowercase.find(recordNameLowercase);
            if (userType == _compiledProgram.userTypesByNameLowercase.end()) {
                throw CompilerException(
                    CompilerErrorCode::kTypeNotFound, fmt::format("Type \"{}\" not found", *typeNode->recordName),
                    typeNode->token);
            }
            assert(typeNode->fields.empty());
            for (auto& field : userType->second->fields) {
                typeNode->fields.push_back(field->parameterNode);
            }
        }
    }

    void bindConvertExpression(ConvertExpressionNode* convertExpressionNode) {
        bindType(convertExpressionNode->type.get());
    }

    void bindExpression(ExpressionNode* expressionNode) {
        switch (expressionNode->getExpressionType()) {
            case ExpressionType::kConvert:
                bindConvertExpression(dynamic_cast<ConvertExpressionNode*>(expressionNode));
                break;

            default:
                break;
        }
    }

    void bindDimStatement(DimStatementNode* dimStatementNode) {
        if (dimStatementNode->type != nullptr) {
            bindType(dimStatementNode->type.get());
        }
    }

    void bindStatement(StatementNode* statementNode) {
        statementNode->visitExpressions(false, [this](ExpressionNode* expressionNode) {
            bindExpression(expressionNode);
            return true;
        });

        switch (statementNode->getStatementType()) {
            case StatementType::kDim:
                bindDimStatement(dynamic_cast<DimStatementNode*>(statementNode));
                break;

            default:
                break;
        }

        statementNode->visitBodies([this](BodyNode* bodyNode) {
            bindBody(bodyNode);
            return true;
        });
    }

    void bindBody(BodyNode* bodyNode) {
        for (auto& statementNode : bodyNode->statements) {
            bindStatement(statementNode.get());
        }
    }

    void bindParameter(ParameterNode* parameterNode) { bindType(parameterNode->type.get()); }

    void bindProcedure(ProcedureNode* procedureNode) {
        for (auto& parameterNode : procedureNode->parameters) {
            bindParameter(parameterNode.get());
        }

        if (procedureNode->returnType != nullptr) {
            bindType(procedureNode->returnType.get());
        }

        bindBody(procedureNode->body.get());
    }
};

void bindNamedRecordTypes(ProcedureNode* procedureNode, const CompiledProgram& compiledProgram) {
    NamedRecordTypeBinder{ compiledProgram }.bindProcedure(procedureNode);
}

void bindNamedRecordTypes(TypeNode* typeNode, const CompiledProgram& compiledProgram) {
    NamedRecordTypeBinder{ compiledProgram }.bindType(typeNode);
}

}  // namespace compiler
