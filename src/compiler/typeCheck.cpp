#include "typeCheck.h"
#include "BuiltInProcedureList.h"
#include "CompilerException.h"
#include "vm/systemCall.h"

namespace compiler {

class TypeCheckState {
   public:
    const SourceProgram& sourceProgram;
    CompiledProgram* compiledProgram;
    const BuiltInProcedureList& builtInProcedures{};
    boost::local_shared_ptr<TypeNode> typeBoolean{ boost::make_local_shared<TypeNode>(Kind::kBoolean, Token{}) };
    boost::local_shared_ptr<TypeNode> typeNumber{ boost::make_local_shared<TypeNode>(Kind::kNumber, Token{}) };
    TypeCheckState(
        const SourceProgram& sourceProgram,
        CompiledProgram* compiledProgram,
        const BuiltInProcedureList& builtInProcedures)
        : sourceProgram(sourceProgram), compiledProgram(compiledProgram), builtInProcedures(builtInProcedures) {}
};

static void typeCheckExpression(ExpressionNode* expressionNode, TypeCheckState* state);

static bool doCallArgumentTypesMatchProcedureParameters(
    const std::vector<std::unique_ptr<ExpressionNode>>& arguments,
    const std::vector<std::unique_ptr<ParameterNode>>& parameters) {
    auto parameterCount = parameters.size();
    auto argumentCount = arguments.size();
    if (parameterCount != argumentCount) {
        return false;
    }

    for (size_t i = 0; i < parameterCount; i++) {
        auto& parameterType = parameters.at(i)->type;
        assert(parameterType != nullptr);
        auto& argumentType = arguments.at(i)->evaluatedType;
        assert(argumentType != nullptr);
        if (!argumentType->canImplicitlyConvertTo(*parameterType)) {
            return false;
        }
    }

    return true;
}

static void typeCheckCall(
    Node* callNode,
    const std::string& name,
    const std::vector<std::unique_ptr<ExpressionNode>>& arguments,
    TypeCheckState* state,
    bool mustBeFunction) {
    for (const auto& argument : arguments) {
        typeCheckExpression(argument.get(), state);
    }

    auto lowercaseProcedureName = boost::to_lower_copy(name);
    for (auto& compiledProcedure : state->compiledProgram->procedures) {
        if (compiledProcedure->nameLowercase == lowercaseProcedureName &&
            doCallArgumentTypesMatchProcedureParameters(arguments, compiledProcedure->procedureNode->parameters)) {
            if (mustBeFunction && compiledProcedure->procedureNode->returnType == nullptr) {
                throw CompilerException(
                    fmt::format("\"{}\" is a subroutine but is being called as a function.", name), callNode->token);
            }
            callNode->procedureIndex = compiledProcedure->procedureIndex;
            if (compiledProcedure->procedureNode->returnType != nullptr) {
                callNode->evaluatedType = compiledProcedure->procedureNode->returnType;
            }
            return;
        }
    }

    for (const auto& builtInProcedure : state->builtInProcedures.get(name)) {
        if (doCallArgumentTypesMatchProcedureParameters(arguments, builtInProcedure->parameters)) {
            if (mustBeFunction && builtInProcedure->returnType == nullptr) {
                throw CompilerException(
                    fmt::format("\"{}\" is a subroutine but is being called as a function.", name), callNode->token);
            }
            callNode->systemCall = builtInProcedure->systemCall;
            if (builtInProcedure->returnType != nullptr) {
                callNode->evaluatedType = builtInProcedure->returnType;
            }
            return;
        }
    }

    throw CompilerException(fmt::format("Call to undefined procedure \"{}\".", name), callNode->token);
}

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

static void typeCheckBinaryExpression(BinaryExpressionNode* expressionNode, TypeCheckState* state) {
    typeCheckExpression(expressionNode->leftOperand.get(), state);
    for (auto& suffix : expressionNode->binarySuffixes) {
        typeCheckExpression(suffix->rightOperand.get(), state);
    }

    auto lhsType = expressionNode->leftOperand->evaluatedType;
    for (const auto& suffix : expressionNode->binarySuffixes) {
        const auto& rhsType = suffix->rightOperand->evaluatedType;

        switch (suffix->binaryOperator) {
            case BinaryOperator::kOr:
            case BinaryOperator::kAnd:
                if (lhsType->kind == Kind::kBoolean && rhsType->kind == Kind::kBoolean) {
                    suffix->evaluatedType = lhsType;
                } else {
                    throw CompilerException(
                        fmt::format(
                            "The \"{}\" operator requires Boolean operands.", getOperatorText(suffix->binaryOperator)),
                        suffix->token);
                }
                lhsType = state->typeBoolean;
                break;

            case BinaryOperator::kEquals:
            case BinaryOperator::kNotEquals:
            case BinaryOperator::kLessThan:
            case BinaryOperator::kLessThanEquals:
            case BinaryOperator::kGreaterThan:
            case BinaryOperator::kGreaterThanEquals: {
                auto identicalTypes = lhsType->isIdentical(*rhsType);
                auto lhsCanConvertToRhs = lhsType->canImplicitlyConvertTo(*rhsType);
                auto rhsCanConvertToLhs = rhsType->canImplicitlyConvertTo(*lhsType);
                if (identicalTypes || lhsCanConvertToRhs || rhsCanConvertToLhs) {
                    suffix->evaluatedType = boost::make_local_shared<TypeNode>(Kind::kBoolean, suffix->token);
                    if (!identicalTypes) {
                        if (lhsCanConvertToRhs) {
                            suffix->leftOperandConvertedType = rhsType;
                        } else if (rhsCanConvertToLhs) {
                            suffix->rightOperandConvertedType = lhsType;
                        } else {
                            assert(false);
                        }
                    }
                } else {
                    throw CompilerException(
                        fmt::format(
                            "The \"{}\" operator requires the operands to have equivalent types.",
                            getOperatorText(suffix->binaryOperator)),
                        suffix->token);
                }
                lhsType = state->typeBoolean;
                break;
            }

            case BinaryOperator::kAdd:
            case BinaryOperator::kSubtract:
            case BinaryOperator::kMultiply:
            case BinaryOperator::kDivide:
            case BinaryOperator::kModulus: {
                // lhs must be Number
                if (lhsType->kind != Kind::kNumber) {
                    throw CompilerException(
                        fmt::format(
                            "The \"{}\" operator requires the left operand to be a Number.",
                            getOperatorText(suffix->binaryOperator)),
                        suffix->token);
                }
                // rhs must be Number
                if (rhsType->kind != Kind::kNumber) {
                    throw CompilerException(
                        fmt::format(
                            "The \"{}\" operator requires the right operand to be a Number.",
                            getOperatorText(suffix->binaryOperator)),
                        suffix->token);
                }
                // if so, the result is also a Number
                suffix->evaluatedType = state->typeNumber;
                break;
            }

            default:
                throw std::runtime_error("not impl");
        }
    }

    expressionNode->evaluatedType = lhsType;
}

static void typeCheckCallExpression(CallExpressionNode* expressionNode, TypeCheckState* state) {
    if (expressionNode->boundSymbolDeclaration != nullptr) {
        const auto& decl = *expressionNode->boundSymbolDeclaration;
        if (dynamic_cast<const ProcedureNode*>(&decl) == nullptr) {
            // this is a variable being used as a function: a map or list
            auto& declType = *decl.getSymbolDeclarationType();
            if (declType.kind == Kind::kMap) {
                // index type must match map key type
                throw std::runtime_error("not impl");
            }
            if (declType.kind == Kind::kList) {
                // index type must be Number
                if (expressionNode->arguments.size() == 1) {
                    typeCheckExpression(expressionNode->arguments.at(0).get(), state);
                    if (expressionNode->arguments.at(0)->evaluatedType->kind == Kind::kNumber) {
                        expressionNode->evaluatedType = declType.listItemType;
                        return;
                    }
                }
                throw CompilerException("The list index must be a single number.", expressionNode->token);
            }
            throw CompilerException(
                "Only lists and maps can be indexed with \"(...)\" like this.", expressionNode->token);
        }
    }

    typeCheckCall(expressionNode, expressionNode->name, expressionNode->arguments, state, true);
}

static void typeCheckConstValueExpressionArray(LiteralArrayExpressionNode* expressionNode, TypeCheckState* state) {
    assert(expressionNode != nullptr);
    if (expressionNode->elements.empty()) {
        throw CompilerException("Literal lists must have at least one element.", expressionNode->token);
    }
    for (auto& element : expressionNode->elements) {
        typeCheckExpression(element.get(), state);
    }
    auto& type = expressionNode->elements.at(0)->evaluatedType;
    for (auto& element : expressionNode->elements) {
        if (!type->isIdentical(*element->evaluatedType)) {
            throw CompilerException("All elements of a literal list must have the same type.", element->token);
        }
    }
    auto arrayType = boost::make_local_shared<TypeNode>(Kind::kList, expressionNode->token, type);
    expressionNode->evaluatedType = arrayType;
}

static void typeCheckConstValueExpressionRecord(LiteralRecordFieldNode* expressionNode) {
    (void)expressionNode;
    throw std::runtime_error("not impl");
}

static void typeCheckConstValueExpression(ConstValueExpressionNode* expressionNode, TypeCheckState* state) {
    switch (expressionNode->getConstValueExpressionType()) {
        case ConstValueExpressionType::kArray:
            typeCheckConstValueExpressionArray(dynamic_cast<LiteralArrayExpressionNode*>(expressionNode), state);
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

static void typeCheckSymbolReferenceExpression(SymbolReferenceExpressionNode* expressionNode, TypeCheckState* state) {
    const auto* decl = expressionNode->boundSymbolDeclaration;
    if (decl == nullptr) {
        throw CompilerException(
            std::string("Internal error. The symbol reference \"") + expressionNode->name +
                "\" is not bound to a symbol declaration.",
            expressionNode->token);
    }
    if (!decl->getSymbolDeclaration().has_value()) {
        std::ostringstream s;
        decl->dump(s, 0);
        throw CompilerException(
            fmt::format(
                "Internal error. The symbol reference \"{}\" is bound to a node that does not claim to declare a "
                "symbol. That node is: {}",
                expressionNode->name, s.str()),
            expressionNode->token);
    }

    // this could be a call to a function with no parameters
    const auto* procedureNode = dynamic_cast<const ProcedureNode*>(decl);
    if (procedureNode != nullptr) {
        std::vector<std::unique_ptr<ExpressionNode>> arguments{};
        typeCheckCall(expressionNode, expressionNode->name, arguments, state, true);
        return;
    }

    // nope, it must be a regular variable declaration
    auto type = decl->getSymbolDeclarationType();
    assert(type != nullptr);
    expressionNode->evaluatedType = std::move(type);
}

void typeCheckExpression(ExpressionNode* expressionNode, TypeCheckState* state) {
    switch (expressionNode->getExpressionType()) {
        case ExpressionType::kBinary:
            typeCheckBinaryExpression(dynamic_cast<BinaryExpressionNode*>(expressionNode), state);
            break;
        case ExpressionType::kCall:
            typeCheckCallExpression(dynamic_cast<CallExpressionNode*>(expressionNode), state);
            break;
        case ExpressionType::kConstValue:
            typeCheckConstValueExpression(dynamic_cast<ConstValueExpressionNode*>(expressionNode), state);
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
            typeCheckSymbolReferenceExpression(dynamic_cast<SymbolReferenceExpressionNode*>(expressionNode), state);
            break;
        default:
            throw std::runtime_error("Unrecognized expression type.");
    }
    // make sure we actually did it
    assert(expressionNode->evaluatedType != nullptr);
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

static void typeCheckIfStatement(IfStatementNode* statementNode) {
    assert(statementNode->condition->evaluatedType != nullptr);
    if (statementNode->condition->evaluatedType->kind != Kind::kBoolean) {
        throw CompilerException(
            "The condition of an \"if\" statement must be a Boolean.", statementNode->condition->token);
    }
    for (auto& elseIf : statementNode->elseIfs) {
        assert(elseIf->condition->evaluatedType != nullptr);
        if (elseIf->condition->evaluatedType->kind != Kind::kBoolean) {
            throw CompilerException(
                "The condition of an \"else if\" statement must be a Boolean.", elseIf->condition->token);
        }
    }
}

static void typeCheckCallStatement(CallStatementNode* statementNode, TypeCheckState* state) {
    typeCheckCall(statementNode, statementNode->name, statementNode->arguments, state, false);
}

static void typeCheckForStatement(ForStatementNode* statementNode) {
    // fromValue must be a number
    assert(statementNode->fromValue->evaluatedType != nullptr);
    if (statementNode->fromValue->evaluatedType->kind != Kind::kNumber) {
        throw CompilerException(
            "The \"from\" value of a \"for\" statement must be a number.", statementNode->fromValue->token);
    }
    // toValue must be a number
    assert(statementNode->toValue->evaluatedType != nullptr);
    if (statementNode->toValue->evaluatedType->kind != Kind::kNumber) {
        throw CompilerException(
            "The \"to\" value of a \"for\" statement must be a number.", statementNode->toValue->token);
    }
    // if it is provided, then step must be a number
    if (statementNode->step != nullptr) {
        assert(statementNode->step->evaluatedType != nullptr);
        if (statementNode->step->evaluatedType->kind != Kind::kNumber) {
            throw CompilerException(
                "The \"step\" value of a \"for\" statement must be a number.", statementNode->step->token);
        }
    }
}

static void typeCheckWhileStatement(WhileStatementNode* statementNode) {
    // condition must be a Boolean
    assert(statementNode->condition->evaluatedType != nullptr);
    if (statementNode->condition->evaluatedType->kind != Kind::kBoolean) {
        throw CompilerException(
            "The condition of a \"while\" statement must be a Boolean.", statementNode->condition->token);
    }
}

static void typeCheckBody(BodyNode* bodyNode, TypeCheckState* state) {
    for (auto& statementNode : bodyNode->statements) {
        statementNode->visitExpressions(true, [state](ExpressionNode* expressionNode) -> bool {
            typeCheckExpression(expressionNode, state);
            return true;
        });

        switch (statementNode->getStatementType()) {
            case StatementType::kDim:
                typeCheckDimStatement(dynamic_cast<DimStatementNode*>(statementNode.get()));
                break;

            case StatementType::kCall:
                typeCheckCallStatement(dynamic_cast<CallStatementNode*>(statementNode.get()), state);
                break;

            case StatementType::kIf:
                typeCheckIfStatement(dynamic_cast<IfStatementNode*>(statementNode.get()));
                break;

            case StatementType::kFor:
                typeCheckForStatement(dynamic_cast<ForStatementNode*>(statementNode.get()));
                break;

            case StatementType::kWhile:
                typeCheckWhileStatement(dynamic_cast<WhileStatementNode*>(statementNode.get()));
                break;

            default:
                // do nothing
                break;
        }

        statementNode->visitBodies([state](BodyNode* innerBodyNode) -> bool {
            typeCheckBody(innerBodyNode, state);
            return true;
        });
    }
}

void typeCheck(
    ProcedureNode* procedureNode,
    const SourceProgram& sourceProgram,
    CompiledProgram* compiledProgram,
    const BuiltInProcedureList& builtInProcedures) {
    TypeCheckState state{ sourceProgram, compiledProgram, builtInProcedures };
    return typeCheckBody(procedureNode->body.get(), &state);
}

};  // namespace compiler
