#include "typeCheck.h"

namespace compiler {

static CompilerResult typeCheckExpression(ExpressionNode* expressionNode);

static std::string getOperatorText(BinaryOperator op) {
    switch (op) {
        case BinaryOperator::kOr:
            return "or";
        case BinaryOperator::kAnd:
            return "and";
        case BinaryOperator::kEquals:
            return "=";
        case BinaryOperator::kNotEquals:
            return "<>";
        case BinaryOperator::kLessThan:
            return "<";
        case BinaryOperator::kLessThanEquals:
            return "<=";
        case BinaryOperator::kGreaterThan:
            return ">";
        case BinaryOperator::kGreaterThanEquals:
            return ">=";
        case BinaryOperator::kAdd:
            return "+";
        case BinaryOperator::kSubtract:
            return "-";
        case BinaryOperator::kMultiply:
            return "*";
        case BinaryOperator::kDivide:
            return "/";
        case BinaryOperator::kModulus:
            return "mod";
        default:
            throw std::runtime_error("not impl");
    }
}

static CompilerResult typeCheckBinaryExpression(BinaryExpressionNode* expressionNode) {
    const auto& lhsType = expressionNode->leftOperand->evaluatedType;
    for (const auto& suffix : expressionNode->binarySuffixes) {
        const auto& rhsType = suffix->rightOperand->evaluatedType;

        switch (suffix->binaryOperator) {
            case BinaryOperator::kOr:
            case BinaryOperator::kAnd:
                if (lhsType->kind == Kind::kBoolean && rhsType->kind == Kind::kBoolean) {
                    expressionNode->evaluatedType = lhsType;
                    return CompilerResult::success();
                } else {
                    return CompilerResult::error(
                        std::string("The \"") + getOperatorText(suffix->binaryOperator) +
                            "\" operator requires operands of type boolean.",
                        suffix->token);
                }

            case BinaryOperator::kEquals:
            case BinaryOperator::kNotEquals:
            case BinaryOperator::kLessThan:
            case BinaryOperator::kLessThanEquals:
            case BinaryOperator::kGreaterThan:
            case BinaryOperator::kGreaterThanEquals:
                if (lhsType->canImplicitlyConvertTo(*rhsType) || rhsType->canImplicitlyConvertTo(*lhsType)) {
                    expressionNode->evaluatedType = boost::make_local_shared<TypeNode>(Kind::kBoolean, suffix->token);
                    return CompilerResult::success();
                } else {
                    return CompilerResult::error(
                        std::string("The \"") + getOperatorText(suffix->binaryOperator) +
                            "\" operator requires boolean operands.",
                        suffix->token);
                }

            case BinaryOperator::kAdd:
                // list + element -> list
                if (lhsType->kind == Kind::kList && rhsType->canImplicitlyConvertTo(*lhsType->listItemType)) {
                    expressionNode->evaluatedType = lhsType;
                    return CompilerResult::success();
                }

            default:
                throw std::runtime_error("not impl");
        }
    }

    throw std::runtime_error("not impl");
}

static CompilerResult typeCheckCallExpression(CallExpressionNode* expressionNode) {
    throw std::runtime_error("not impl");
}

static CompilerResult typeCheckConstValueExpression(ConstValueExpressionNode* expressionNode) {
    throw std::runtime_error("not impl");
}

static CompilerResult typeCheckConvertExpression(ConvertExpressionNode* expressionNode) {
    throw std::runtime_error("not impl");
}

static CompilerResult typeCheckDottedExpression(DottedExpressionNode* expressionNode) {
    throw std::runtime_error("not impl");
}

static CompilerResult typeCheckNotExpression(NotExpressionNode* expressionNode) {
    throw std::runtime_error("not impl");
}

static CompilerResult typeCheckSymbolReferenceExpression(SymbolReferenceExpressionNode* expressionNode) {
    throw std::runtime_error("not impl");
}

CompilerResult typeCheckExpression(ExpressionNode* expressionNode) {
    switch (expressionNode->getExpressionType()) {
        case ExpressionType::kBinary:
            return typeCheckBinaryExpression(dynamic_cast<BinaryExpressionNode*>(expressionNode));
        case ExpressionType::kCall:
            return typeCheckCallExpression(dynamic_cast<CallExpressionNode*>(expressionNode));
        case ExpressionType::kConstValue:
            return typeCheckConstValueExpression(dynamic_cast<ConstValueExpressionNode*>(expressionNode));
        case ExpressionType::kConvert:
            return typeCheckConvertExpression(dynamic_cast<ConvertExpressionNode*>(expressionNode));
        case ExpressionType::kDotted:
            return typeCheckDottedExpression(dynamic_cast<DottedExpressionNode*>(expressionNode));
        case ExpressionType::kNot:
            return typeCheckNotExpression(dynamic_cast<NotExpressionNode*>(expressionNode));
        case ExpressionType::kSymbolReference:
            return typeCheckSymbolReferenceExpression(dynamic_cast<SymbolReferenceExpressionNode*>(expressionNode));
        default:
            assert(false);
            throw std::runtime_error("Unrecognized expression type.");
    }
}

static CompilerResult typeCheckBody(BodyNode* bodyNode) {
    auto result = CompilerResult::success();

    for (auto& statementNode : bodyNode->statements) {
        statementNode->visitExpressions([&result](ExpressionNode& expressionNode) -> bool {
            result = typeCheckExpression(&expressionNode);
            return result.isSuccess;
        });
        if (!result.isSuccess) {
            return result;
        }
    }

    return result;
}

CompilerResult typeCheck(ProcedureNode* procedureNode) {
    return typeCheckBody(procedureNode->body.get());
}

};  // namespace compiler
