#include "typeCheck.h"
#include "CompilerException.h"

namespace compiler {

static void typeCheckExpression(ExpressionNode* expressionNode);

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

static void typeCheckBinaryExpression(BinaryExpressionNode* expressionNode) {
    const auto& lhsType = expressionNode->leftOperand->evaluatedType;
    for (const auto& suffix : expressionNode->binarySuffixes) {
        const auto& rhsType = suffix->rightOperand->evaluatedType;

        switch (suffix->binaryOperator) {
            case BinaryOperator::kOr:
            case BinaryOperator::kAnd:
                if (lhsType->kind == Kind::kBoolean && rhsType->kind == Kind::kBoolean) {
                    expressionNode->evaluatedType = lhsType;
                    return;
                } else {
                    throw CompilerException(
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
                    return;
                } else {
                    throw CompilerException(
                        std::string("The \"") + getOperatorText(suffix->binaryOperator) +
                            "\" operator requires boolean operands.",
                        suffix->token);
                }

            case BinaryOperator::kAdd:
                // list + element -> list
                if (lhsType->kind == Kind::kList && rhsType->canImplicitlyConvertTo(*lhsType->listItemType)) {
                    expressionNode->evaluatedType = lhsType;
                    return;
                }

            default:
                throw std::runtime_error("not impl");
        }
    }

    throw std::runtime_error("not impl");
}

static void typeCheckCallExpression(CallExpressionNode* expressionNode) {
    (void)expressionNode;
    throw std::runtime_error("not impl");
}

static void typeCheckConstValueExpressionArray(LiteralArrayExpressionNode* expressionNode) {
    (void)expressionNode;
    throw std::runtime_error("not impl");
}

static void typeCheckConstValueExpressionRecord(LiteralRecordFieldNode* expressionNode) {
    (void)expressionNode;
    throw std::runtime_error("not impl");
}

static void typeCheckConstValueExpression(ConstValueExpressionNode* expressionNode) {
    switch (expressionNode->getConstValueExpressionType()) {
        case ConstValueExpressionType::kArray:
            typeCheckConstValueExpressionArray(dynamic_cast<LiteralArrayExpressionNode*>(expressionNode));
            break;
        case ConstValueExpressionType::kBoolean:
            expressionNode->evaluatedType = boost::make_local_shared<TypeNode>(Kind::kBoolean, expressionNode->token);
            break;
        case ConstValueExpressionType::kNumber:
            expressionNode->evaluatedType = boost::make_local_shared<TypeNode>(Kind::kNumber, expressionNode->token);
            break;
        case ConstValueExpressionType::kRecord:
            typeCheckConstValueExpressionRecord(dynamic_cast<LiteralRecordFieldNode*>(expressionNode));
            break;
        case ConstValueExpressionType::kString:
            expressionNode->evaluatedType = boost::make_local_shared<TypeNode>(Kind::kString, expressionNode->token);
            break;
        default:
            assert(false);
            throw std::runtime_error("Unrecognized ConstValueExpressionType.");
    }
}

static void typeCheckConvertExpression(ConvertExpressionNode* expressionNode) {
    (void)expressionNode;
    throw std::runtime_error("not impl");
}

static void typeCheckDottedExpression(DottedExpressionNode* expressionNode) {
    (void)expressionNode;
    throw std::runtime_error("not impl");
}

static void typeCheckNotExpression(NotExpressionNode* expressionNode) {
    (void)expressionNode;
    throw std::runtime_error("not impl");
}

static void typeCheckSymbolReferenceExpression(SymbolReferenceExpressionNode* expressionNode) {
    (void)expressionNode;
    throw std::runtime_error("not impl");
}

void typeCheckExpression(ExpressionNode* expressionNode) {
    switch (expressionNode->getExpressionType()) {
        case ExpressionType::kBinary:
            typeCheckBinaryExpression(dynamic_cast<BinaryExpressionNode*>(expressionNode));
        case ExpressionType::kCall:
            typeCheckCallExpression(dynamic_cast<CallExpressionNode*>(expressionNode));
        case ExpressionType::kConstValue:
            typeCheckConstValueExpression(dynamic_cast<ConstValueExpressionNode*>(expressionNode));
        case ExpressionType::kConvert:
            typeCheckConvertExpression(dynamic_cast<ConvertExpressionNode*>(expressionNode));
        case ExpressionType::kDotted:
            typeCheckDottedExpression(dynamic_cast<DottedExpressionNode*>(expressionNode));
        case ExpressionType::kNot:
            typeCheckNotExpression(dynamic_cast<NotExpressionNode*>(expressionNode));
        case ExpressionType::kSymbolReference:
            typeCheckSymbolReferenceExpression(dynamic_cast<SymbolReferenceExpressionNode*>(expressionNode));
        default:
            assert(false);
            throw std::runtime_error("Unrecognized expression type.");
    }
}

static void typeCheckBody(BodyNode* bodyNode) {
    for (auto& statementNode : bodyNode->statements) {
        statementNode->visitExpressions([](ExpressionNode& expressionNode) -> bool {
            typeCheckExpression(&expressionNode);
            return true;
        });
    }
}

void typeCheck(ProcedureNode* procedureNode) {
    return typeCheckBody(procedureNode->body.get());
}

};  // namespace compiler
