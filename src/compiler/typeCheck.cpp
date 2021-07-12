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
                break;

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
    auto* decl = expressionNode->boundSymbolDeclaration;
    if (decl == nullptr) {
        throw CompilerException(
            std::string("Internal error. The symbol reference \"") + expressionNode->name +
                "\" is not bound to a symbol declaration.",
            expressionNode->token);
    }
    if (!decl->getSymbolDeclaration().has_value()) {
        std::ostringstream s;
        decl->dump(s, 0);
        throw CompilerException(  // NAMEOF_TYPE_RTTI
            fmt::format(
                "Internal error. The symbol reference \"{}\" is bound to a node that does not claim to declare a "
                "symbol. That node is: {}",
                expressionNode->name, s.str()),
            expressionNode->token);
    }
    auto type = decl->getSymbolDeclarationType();
    assert(type != nullptr);
    expressionNode->evaluatedType = std::move(type);
}

void typeCheckExpression(ExpressionNode* expressionNode) {
    switch (expressionNode->getExpressionType()) {
        case ExpressionType::kBinary:
            typeCheckBinaryExpression(dynamic_cast<BinaryExpressionNode*>(expressionNode));
            break;
        case ExpressionType::kCall:
            typeCheckCallExpression(dynamic_cast<CallExpressionNode*>(expressionNode));
            break;
        case ExpressionType::kConstValue:
            typeCheckConstValueExpression(dynamic_cast<ConstValueExpressionNode*>(expressionNode));
            break;
        case ExpressionType::kConvert:
            typeCheckConvertExpression(dynamic_cast<ConvertExpressionNode*>(expressionNode));
            break;
        case ExpressionType::kDotted:
            typeCheckDottedExpression(dynamic_cast<DottedExpressionNode*>(expressionNode));
            break;
        case ExpressionType::kNot:
            typeCheckNotExpression(dynamic_cast<NotExpressionNode*>(expressionNode));
            break;
        case ExpressionType::kSymbolReference:
            typeCheckSymbolReferenceExpression(dynamic_cast<SymbolReferenceExpressionNode*>(expressionNode));
            break;
        default:
            assert(false);
            throw std::runtime_error("Unrecognized expression type.");
    }
}

static void typeCheckDimStatement(DimStatementNode* statementNode) {
    if (statementNode->type == nullptr) {
        assert(statementNode->value != nullptr);
        assert(statementNode->value->evaluatedType != nullptr);
        statementNode->evaluatedType = statementNode->value->evaluatedType;
    } else {
        statementNode->evaluatedType = statementNode->type;
    }
}

static void typeCheckBody(BodyNode* bodyNode) {
    for (auto& statementNode : bodyNode->statements) {
        statementNode->visitExpressions([](ExpressionNode& expressionNode) -> bool {
            typeCheckExpression(&expressionNode);
            return true;
        });

        switch (statementNode->getStatementType()) {
            case StatementType::kDim:
                typeCheckDimStatement(dynamic_cast<DimStatementNode*>(statementNode.get()));
                break;

            default:
                // do nothing
                break;
        }
    }
}

void typeCheck(ProcedureNode* procedureNode) {
    return typeCheckBody(procedureNode->body.get());
}

};  // namespace compiler
