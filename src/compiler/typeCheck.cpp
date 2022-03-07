#include "typeCheck.h"
#include "BuiltInProcedureList.h"
#include "CompilerException.h"
#include "vm/systemCall.h"

namespace compiler {

class TypeCheckState {
   public:
    const SourceProgram& sourceProgram;
    CompiledProgram* compiledProgram;
    const BuiltInProcedureList& builtInProcedures;
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

        // parameter may be generic:
        // List of Any
        // Map of Any to Any
        // Optional Any
        if (parameterType->kind == Kind::kList && argumentType->kind == Kind::kList &&
            parameterType->listItemType->kind == Kind::kAny) {
            continue;
        }
        if (parameterType->kind == Kind::kMap && argumentType->kind == Kind::kMap &&
            parameterType->mapKeyType->kind == Kind::kAny && parameterType->mapValueType->kind == Kind::kAny) {
            continue;
        }
        if (parameterType->kind == Kind::kOptional && argumentType->kind == Kind::kOptional &&
            parameterType->optionalValueType->kind == Kind::kAny) {
            continue;
        }

        if (!argumentType->equals(*parameterType)) {
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

    auto anyNameMatches = false;

    // Note: fixDottedExpressionFunctionCalls() and typeCheckCall() duplicate the procedure lookup logic

    auto lowercaseProcedureName = boost::to_lower_copy(name);
    for (auto& compiledProcedure : state->compiledProgram->procedures) {
        if (compiledProcedure->nameLowercase == lowercaseProcedureName) {
            anyNameMatches = true;
            if (doCallArgumentTypesMatchProcedureParameters(arguments, compiledProcedure->procedureNode->parameters)) {
                if (mustBeFunction && compiledProcedure->procedureNode->returnType == nullptr) {
                    throw CompilerException(
                        CompilerErrorCode::kSubCalledAsFunction,
                        fmt::format("\"{}\" is a subroutine but is being called as a function.", name),
                        callNode->token);
                }
                callNode->procedureIndex = compiledProcedure->procedureIndex;
                if (compiledProcedure->procedureNode->returnType != nullptr) {
                    callNode->evaluatedType = compiledProcedure->procedureNode->returnType;
                }
                return;
            }
        }
    }

    for (const auto& builtInProcedure : state->builtInProcedures.get(name)) {
        anyNameMatches = true;
        if (doCallArgumentTypesMatchProcedureParameters(arguments, builtInProcedure->parameters)) {
            if (mustBeFunction && builtInProcedure->returnType == nullptr) {
                throw CompilerException(
                    CompilerErrorCode::kSubCalledAsFunction,
                    fmt::format("\"{}\" is a subroutine but is being called as a function.", name), callNode->token);
            }
            callNode->systemCall = builtInProcedure->systemCall;
            if (builtInProcedure->returnType != nullptr) {
                callNode->evaluatedType = builtInProcedure->returnType;
            }
            return;
        }
    }

    throw CompilerException(
        CompilerErrorCode::kProcedureNotFound,
        anyNameMatches ? fmt::format("Call to procedure \"{}\" with the wrong parameters.", name)
                       : fmt::format("Call to undefined procedure \"{}\".", name),
        callNode->token);
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
        case BinaryOperator::kPower:
            return "^";
        default:
            throw CompilerException(
                CompilerErrorCode::kInternal, fmt::format("Internal error. Unimplemented binary operator."), Token{});
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
                        CompilerErrorCode::kTypeMismatch,
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
                if (lhsType->equals(*rhsType)) {
                    suffix->evaluatedType = boost::make_local_shared<TypeNode>(Kind::kBoolean, suffix->token);
                } else {
                    throw CompilerException(
                        CompilerErrorCode::kTypeMismatch,
                        fmt::format(
                            "The \"{}\" operator requires the operands to have identical types.",
                            getOperatorText(suffix->binaryOperator)),
                        suffix->token);
                }
                lhsType = state->typeBoolean;
                break;
            }

            case BinaryOperator::kAdd:
                if (lhsType->kind == Kind::kNumber && rhsType->kind == Kind::kNumber) {
                    suffix->evaluatedType = lhsType;
                } else if (lhsType->kind == Kind::kString && rhsType->kind == Kind::kString) {
                    suffix->evaluatedType = lhsType;
                } else if (lhsType->kind == Kind::kList && rhsType->equals(*lhsType->listItemType)) {
                    suffix->evaluatedType = lhsType;
                } else if (lhsType->kind == Kind::kList && rhsType->kind == Kind::kList) {
                    if (lhsType->listItemType->equals(*rhsType->listItemType)) {
                        suffix->evaluatedType = lhsType;
                    } else {
                        throw CompilerException(
                            CompilerErrorCode::kTypeMismatch,
                            fmt::format(
                                "These lists cannot be combined because the list on the left is type {} and the list "
                                "on the right is type {}. The types must match.",
                                lhsType->listItemType->toString(), rhsType->listItemType->toString()),
                            suffix->token);
                    }
                } else {
                    throw CompilerException(
                        CompilerErrorCode::kTypeMismatch,
                        fmt::format(
                            "The types {} and {} are not valid operands for the \"+\" operator.", lhsType->toString(),
                            rhsType->toString()),
                        suffix->token);
                }
                break;

            case BinaryOperator::kSubtract:
            case BinaryOperator::kMultiply:
            case BinaryOperator::kDivide:
            case BinaryOperator::kModulus:
            case BinaryOperator::kPower:
                // lhs must be Number
                if (lhsType->kind != Kind::kNumber) {
                    throw CompilerException(
                        CompilerErrorCode::kTypeMismatch,
                        fmt::format(
                            "The \"{}\" operator requires the left operand to be a Number.",
                            getOperatorText(suffix->binaryOperator)),
                        suffix->token);
                }
                // rhs must be Number
                if (rhsType->kind != Kind::kNumber) {
                    throw CompilerException(
                        CompilerErrorCode::kTypeMismatch,
                        fmt::format(
                            "The \"{}\" operator requires the right operand to be a Number.",
                            getOperatorText(suffix->binaryOperator)),
                        suffix->token);
                }
                // if so, the result is also a Number
                suffix->evaluatedType = state->typeNumber;
                break;

            default:
                throw CompilerException(
                    CompilerErrorCode::kInternal,
                    fmt::format("Internal error. Unknown binary operator {}", static_cast<int>(suffix->binaryOperator)),
                    suffix->token);
        }
    }

    expressionNode->evaluatedType = lhsType;
}

static void typeCheckConstValueExpressionArray(LiteralArrayExpressionNode* expressionNode, TypeCheckState* state) {
    assert(expressionNode != nullptr);
    if (expressionNode->elements.empty()) {
        throw CompilerException(
            CompilerErrorCode::kEmptyLiteralList, "Literal lists must have at least one element.",
            expressionNode->token);
    }
    for (auto& element : expressionNode->elements) {
        typeCheckExpression(element.get(), state);
    }
    auto& type = expressionNode->elements.at(0)->evaluatedType;
    for (auto& element : expressionNode->elements) {
        if (!type->equals(*element->evaluatedType)) {
            throw CompilerException(
                CompilerErrorCode::kTypeMismatch, "All elements of a literal list must have the same type.",
                element->token);
        }
    }
    auto arrayType = boost::make_local_shared<TypeNode>(Kind::kList, expressionNode->token, type);
    expressionNode->evaluatedType = arrayType;
}

static void typeCheckLiteralRecordExpressionNode(LiteralRecordExpressionNode* expressionNode, TypeCheckState* state) {
    assert(expressionNode != nullptr);
    std::vector<boost::local_shared_ptr<ParameterNode>> typeFields;
    for (auto& field : expressionNode->fields) {
        assert(field != nullptr);
        typeCheckExpression(field->value.get(), state);
        typeFields.push_back(
            boost::make_local_shared<ParameterNode>(field->key, field->value->evaluatedType, field->value->token));
    }
    expressionNode->evaluatedType =
        boost::make_local_shared<TypeNode>(Kind::kRecord, expressionNode->token, typeFields);
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
            typeCheckLiteralRecordExpressionNode(dynamic_cast<LiteralRecordExpressionNode*>(expressionNode), state);
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

static void typeCheckFunctionCallExpression(FunctionCallExpressionNode* expressionNode, TypeCheckState* state) {
    typeCheckCall(expressionNode, expressionNode->name, expressionNode->args, state, true);
}

static void typeCheckDottedExpression(DottedExpressionNode* expressionNode, TypeCheckState* state) {
    typeCheckExpression(expressionNode->base.get(), state);
    auto baseType = expressionNode->base->evaluatedType;
    auto baseSymbolRef = dynamic_cast<SymbolReferenceExpressionNode*>(expressionNode->base.get());
    auto isFirstSuffix = true;

    for (auto& usageSuffix : expressionNode->dottedSuffixes) {
        if (usageSuffix->isFieldAccess()) {
            // this is a record field access like "base.suffix"
            if (baseType->kind != Kind::kRecord) {
                throw CompilerException(
                    CompilerErrorCode::kTypeMismatch,
                    fmt::format(
                        "The base of a dotted expression must be a Record type, but this is a {}.",
                        baseType->toString()),
                    baseType->token);
            }

            assert(!(*usageSuffix->nameLowercase).empty());
            ParameterNode* typeField{};
            for (auto& f : baseType->fields) {
                assert(!f->nameLowercase.empty());
                if (f->nameLowercase == usageSuffix->nameLowercase) {
                    typeField = f.get();
                }
            }

            if (typeField == nullptr) {
                throw CompilerException(
                    CompilerErrorCode::kFieldNotFound,
                    fmt::format(
                        "The type {} does not have a field named \"{}\".", baseType->toString(), *usageSuffix->name),
                    usageSuffix->token);
            }

            assert(typeField->fieldValueIndex.has_value() || typeField->fieldObjectIndex.has_value());
            usageSuffix->boundParameterNode = typeField;
            baseType = typeField->type;
            usageSuffix->evaluatedType = baseType;
        } else if (usageSuffix->isIndexOrCall()) {
            if (isFirstSuffix && baseSymbolRef->boundSymbolDeclaration != nullptr &&
                baseSymbolRef->boundSymbolDeclaration->getMemberType() == MemberType::kProcedure) {
                // this is a function call
                typeCheckCall(
                    usageSuffix.get(), baseSymbolRef->name, usageSuffix->collectionIndexOrCallArgs, state, true);
                assert(usageSuffix->evaluatedType != nullptr);

                // set baseType to the return type of the function
                baseType = usageSuffix->evaluatedType;
            } else {
                if (usageSuffix->collectionIndexOrCallArgs.size() != 1) {
                    throw CompilerException(
                        CompilerErrorCode::kInvalidListIndex,
                        fmt::format(
                            "The parentheses here look like a List or Map index, but there are {} comma-separated "
                            "expressions inside these parentheses instead of 1.",
                            usageSuffix->collectionIndexOrCallArgs.size()),
                        usageSuffix->token);
                }

                if (baseType->kind != Kind::kList && baseType->kind != Kind::kMap) {
                    throw CompilerException(
                        CompilerErrorCode::kTypeMismatch,
                        fmt::format(
                            "The base of an index expression must be a List or Map type, but this is a {}.",
                            baseType->toString()),
                        baseType->token);
                }

                auto& collectionIndex = usageSuffix->collectionIndexOrCallArgs.at(0);

                typeCheckExpression(collectionIndex.get(), state);
                if (baseType->kind == Kind::kList) {
                    if (collectionIndex->evaluatedType->kind != Kind::kNumber) {
                        throw CompilerException(
                            CompilerErrorCode::kTypeMismatch,
                            fmt::format(
                                "The list index must be a Number, but this is a {}.",
                                collectionIndex->evaluatedType->toString()),
                            collectionIndex->token);
                    }
                    baseType = baseType->listItemType;
                } else if (baseType->kind == Kind::kMap) {
                    assert(baseType->mapKeyType != nullptr);
                    if (!collectionIndex->evaluatedType->equals(*baseType->mapKeyType)) {
                        throw CompilerException(
                            CompilerErrorCode::kTypeMismatch,
                            fmt::format(
                                "The map key must be a {}, but this is a {}.", baseType->mapKeyType->toString(),
                                collectionIndex->evaluatedType->toString()),
                            collectionIndex->token);
                    }
                    baseType = baseType->mapValueType;
                }
            }

            usageSuffix->evaluatedType = baseType;
        } else {
            assert(false);
        }

        isFirstSuffix = false;
    }

    expressionNode->evaluatedType = baseType;
}

static void typeCheckNotExpression(NotExpressionNode* expressionNode, TypeCheckState* state) {
    typeCheckExpression(expressionNode->operand.get(), state);
    assert(expressionNode->operand->evaluatedType != nullptr);
    auto& operandType = expressionNode->operand->evaluatedType;
    if (operandType->kind != Kind::kBoolean) {
        throw CompilerException(
            CompilerErrorCode::kTypeMismatch, "The \"not\" operator requires a Boolean operand.",
            expressionNode->operand->token);
    }
    expressionNode->evaluatedType = operandType;
}

static void typeCheckSymbolReferenceExpression(SymbolReferenceExpressionNode* expressionNode, TypeCheckState* state) {
    const auto* decl = expressionNode->boundSymbolDeclaration;
    assert(decl != nullptr);
    assert(decl->getSymbolDeclaration().has_value());

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
        case ExpressionType::kConstValue:
            typeCheckConstValueExpression(dynamic_cast<ConstValueExpressionNode*>(expressionNode), state);
            break;
        case ExpressionType::kConvert:
            typeCheckConvertExpression(dynamic_cast<ConvertExpressionNode*>(expressionNode));
            break;
        case ExpressionType::kDotted:
            typeCheckDottedExpression(dynamic_cast<DottedExpressionNode*>(expressionNode), state);
            break;
        case ExpressionType::kNot:
            typeCheckNotExpression(dynamic_cast<NotExpressionNode*>(expressionNode), state);
            break;
        case ExpressionType::kSymbolReference:
            typeCheckSymbolReferenceExpression(dynamic_cast<SymbolReferenceExpressionNode*>(expressionNode), state);
            break;
        case ExpressionType::kFunctionCall:
            typeCheckFunctionCallExpression(dynamic_cast<FunctionCallExpressionNode*>(expressionNode), state);
            break;
        default:
            throw std::runtime_error("Unrecognized expression type.");
    }
    // make sure we actually did it
    assert(expressionNode->evaluatedType != nullptr);
}

static void typeCheckAssignToExpression(const ExpressionNode& expressionNode);

static void typeCheckAssignToDottedExpression(const DottedExpressionNode& expressionNode) {
    typeCheckAssignToExpression(*expressionNode.base);
}

static void typeCheckAssignToSymbolReferenceExpression(const SymbolReferenceExpressionNode& expressionNode) {
    assert(expressionNode.boundSymbolDeclaration != nullptr);
    const auto& decl = *expressionNode.boundSymbolDeclaration;
    if (decl.procedureIndex.has_value() || decl.systemCall.has_value()) {
        throw CompilerException(
            CompilerErrorCode::kTypeMismatch, "Cannot assign to a function call.", expressionNode.token);
    }

    if (decl.getMemberType() == MemberType::kConstStatement) {
        throw CompilerException(
            CompilerErrorCode::kInvalidAssignmentTarget, "Cannot assign to a constant.", expressionNode.token);
    }
}

/*static*/ void typeCheckAssignToExpression(const ExpressionNode& expressionNode) {
    switch (expressionNode.getExpressionType()) {
        case ExpressionType::kDotted:
            typeCheckAssignToDottedExpression(dynamic_cast<const DottedExpressionNode&>(expressionNode));
            break;
        case ExpressionType::kSymbolReference:
            typeCheckAssignToSymbolReferenceExpression(
                dynamic_cast<const SymbolReferenceExpressionNode&>(expressionNode));
            break;
        default:
            throw CompilerException(
                CompilerErrorCode::kInvalidAssignmentTarget,
                "The target of an assignment must be a variable, field access, or collection element.",
                expressionNode.token);
    }
}

static void typeCheckAssignStatement(AssignStatementNode* statementNode) {
    typeCheckAssignToExpression(*statementNode->target);

    // the LHS and RHS must have matching types
    assert(statementNode->target->evaluatedType != nullptr);
    assert(statementNode->value->evaluatedType != nullptr);
    if (!statementNode->target->evaluatedType->equals(*statementNode->value->evaluatedType)) {
        throw CompilerException(
            CompilerErrorCode::kTypeMismatch,
            fmt::format(
                "The types on the left-hand and right-hand sides of an assignment must match, but these types are {} "
                "and {}.",
                statementNode->target->evaluatedType->toString(), statementNode->value->evaluatedType->toString()),
            statementNode->token);
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

static void typeCheckConstStatement(ConstStatementNode* statementNode) {
    assert(statementNode->value != nullptr);
    assert(statementNode->value->evaluatedType != nullptr);
    statementNode->evaluatedType = statementNode->value->evaluatedType;
}

static void typeCheckIfStatement(IfStatementNode* statementNode) {
    assert(statementNode->condition->evaluatedType != nullptr);
    if (statementNode->condition->evaluatedType->kind != Kind::kBoolean) {
        throw CompilerException(
            CompilerErrorCode::kTypeMismatch, "The condition of an \"if\" statement must be a Boolean.",
            statementNode->condition->token);
    }
    for (auto& elseIf : statementNode->elseIfs) {
        assert(elseIf->condition->evaluatedType != nullptr);
        if (elseIf->condition->evaluatedType->kind != Kind::kBoolean) {
            throw CompilerException(
                CompilerErrorCode::kTypeMismatch, "The condition of an \"else if\" statement must be a Boolean.",
                elseIf->condition->token);
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
            CompilerErrorCode::kTypeMismatch, "The \"from\" value of a \"for\" statement must be a number.",
            statementNode->fromValue->token);
    }
    // toValue must be a number
    assert(statementNode->toValue->evaluatedType != nullptr);
    if (statementNode->toValue->evaluatedType->kind != Kind::kNumber) {
        throw CompilerException(
            CompilerErrorCode::kTypeMismatch, "The \"to\" value of a \"for\" statement must be a number.",
            statementNode->toValue->token);
    }
    // if it is provided, then step must be a number
    if (statementNode->step != nullptr) {
        assert(statementNode->step->evaluatedType != nullptr);
        if (statementNode->step->evaluatedType->kind != Kind::kNumber) {
            throw CompilerException(
                CompilerErrorCode::kTypeMismatch, "The \"step\" value of a \"for\" statement must be a number.",
                statementNode->step->token);
        }
    }
}

static void typeCheckForEachStatement(ForEachStatementNode* statementNode) {
    // haystack must be a List
    assert(statementNode->haystack->evaluatedType != nullptr);
    if (statementNode->haystack->evaluatedType->kind != Kind::kList) {
        throw CompilerException(
            CompilerErrorCode::kTypeMismatch, "The collection argument of a \"for each\" statement must be a List.",
            statementNode->haystack->token);
    }
}

static void typeCheckWhileStatement(WhileStatementNode* statementNode) {
    // condition must be a Boolean
    assert(statementNode->condition->evaluatedType != nullptr);
    if (statementNode->condition->evaluatedType->kind != Kind::kBoolean) {
        throw CompilerException(
            CompilerErrorCode::kTypeMismatch, "The condition of a \"while\" statement must be a Boolean.",
            statementNode->condition->token);
    }
}

static void typeCheckDoStatement(DoStatementNode* statementNode) {
    // condition must be a Boolean
    assert(statementNode->condition->evaluatedType != nullptr);
    if (statementNode->condition->evaluatedType->kind != Kind::kBoolean) {
        throw CompilerException(
            CompilerErrorCode::kTypeMismatch, "The condition of a \"do\" statement must be a Boolean.",
            statementNode->condition->token);
    }
}

static void typeCheckThrowStatement(ThrowStatementNode* statementNode) {
    // message must be a String
    assert(statementNode->message->evaluatedType != nullptr);
    if (statementNode->message->evaluatedType->kind != Kind::kString) {
        throw CompilerException(
            CompilerErrorCode::kTypeMismatch, "The error message of a \"throw\" statement must be a String.",
            statementNode->message->token);
    }
    // if code is non-null, then it must be a Number
    if (statementNode->code != nullptr) {
        assert(statementNode->code->evaluatedType != nullptr);
        if (statementNode->code->evaluatedType->kind != Kind::kNumber) {
            throw CompilerException(
                CompilerErrorCode::kTypeMismatch, "The error code of a \"throw\" statement must be a Number.",
                statementNode->code->token);
        }
    }
}

static void typeCheckSelectCaseStatement(SelectCaseStatementNode* statementNode) {
    assert(statementNode->expression->evaluatedType != nullptr);
    auto& exprType = *statementNode->expression->evaluatedType;

    // 1. every case must have expressions that match the switch type
    // 2. case range expressions like "1 to 5" must be numbers
    // 3. there must be at most one "case else"
    bool seenCaseElse = false;
    for (auto& caseNode : statementNode->cases) {
        if (caseNode->values.empty()) {
            if (seenCaseElse) {
                throw CompilerException(
                    CompilerErrorCode::kMultipleSelectCaseDefaults,
                    "This \"select case\" has multiple \"case else\" blocks, but it can only have one.",
                    caseNode->token);
            }
            seenCaseElse = true;
        }

        for (auto& caseValueNode : caseNode->values) {
            assert(caseValueNode->expression->evaluatedType != nullptr);
            auto& caseValueType = *caseValueNode->expression->evaluatedType;
            if (!caseValueNode->expression->evaluatedType->equals(exprType)) {
                throw CompilerException(
                    CompilerErrorCode::kTypeMismatch,
                    fmt::format(
                        "This \"case\" expression is of type {}, which does not match the \"select case\" expression "
                        "type {}.",
                        caseValueType.toString(), exprType.toString()),
                    caseValueNode->expression->token);
            }

            if (caseValueNode->toExpression != nullptr) {
                if (caseValueType.kind != Kind::kNumber) {
                    throw CompilerException(
                        CompilerErrorCode::kTypeMismatch,
                        fmt::format(
                            "Ranges in \"case\" expressions must be numbers, but this range's start value is of type "
                            "{}.",
                            caseValueType.toString()),
                        caseValueNode->expression->token);
                }

                auto& toType = *caseValueNode->toExpression->evaluatedType;
                if (toType.kind != Kind::kNumber) {
                    throw CompilerException(
                        CompilerErrorCode::kTypeMismatch,
                        fmt::format(
                            "Ranges in \"case\" expressions must be numbers, but this range's end value is of type {}.",
                            toType.toString()),
                        caseValueNode->toExpression->token);
                }
            }
        }
    }
}

static void typeCheckYieldStatement(YieldStatementNode* statementNode) {
    auto isListYield = statementNode->toExpression == nullptr;
    auto isMapYield = !isListYield;
    assert(statementNode->boundCollectionDeclaration != nullptr);
    auto* procedure = dynamic_cast<ProcedureNode*>(statementNode->boundCollectionDeclaration);

    if (procedure != nullptr) {
        if (procedure->returnType == nullptr) {
            throw CompilerException(
                CompilerErrorCode::kYieldInSubroutine,
                "A subroutine cannot contain a \"yield\" statement. It must be a function that returns a List or Map.",
                statementNode->boundCollectionDeclaration->token);
        }
        if (procedure->returnType->kind != Kind::kList && procedure->returnType->kind != Kind::kMap) {
            throw CompilerException(
                CompilerErrorCode::kTypeMismatch,
                fmt::format(
                    "This function returns type {}, but the \"yield\" statement requires a List or Map return type.",
                    procedure->returnType->toString()),
                statementNode->boundCollectionDeclaration->token);
        }
        auto* collectionType = procedure->returnType.get();
        assert(collectionType != nullptr);

        if (collectionType->kind == Kind::kList && !isListYield) {
            throw CompilerException(
                CompilerErrorCode::kTypeMismatch,
                fmt::format(
                    "This \"yield ... to ...\" statement syntax is for building Map collections. However, this "
                    "collection is a {}.",
                    collectionType->toString()),
                statementNode->token);
        }

        if (collectionType->kind == Kind::kMap && !isMapYield) {
            throw CompilerException(
                CompilerErrorCode::kTypeMismatch,
                fmt::format(
                    "This \"yield\" statement syntax is for building Map collections. However, this collection is a "
                    "{}.",
                    collectionType->toString()),
                statementNode->token);
        }

        if (collectionType->kind == Kind::kList) {
            auto& procedureItemType = procedure->returnType->listItemType;
            if (!procedureItemType->equals(*statementNode->expression->evaluatedType)) {
                throw CompilerException(
                    CompilerErrorCode::kTypeMismatch,
                    fmt::format(
                        "The expression in this \"yield\" statement must be of type {}, but it is type {}.",
                        procedureItemType->toString(), statementNode->expression->evaluatedType->toString()),
                    statementNode->expression->token);
            }
        } else {
            throw std::runtime_error("not impl");
        }
    }
}

static void typeCheckDimListStatement(DimListStatementNode* statementNode) {
    // there must be at least one yield statement
    auto& yields = *statementNode->getYieldStatementNodesList();
    if (yields.empty()) {
        throw CompilerException(
            CompilerErrorCode::kNoYieldsInDimCollection,
            "A \"dim list\" block must contain at least one \"yield\" statement.", statementNode->token);
    }

    // all yields must be of the same type
    auto& firstType = yields.at(0)->expression->evaluatedType;
    assert(firstType != nullptr);
    for (auto& yieldNode : yields) {
        auto* yieldType = yieldNode->expression->evaluatedType.get();
        if (!yieldType->equals(*firstType)) {
            throw CompilerException(
                CompilerErrorCode::kTypeMismatch,
                fmt::format(
                    "All \"yield\" statements in a \"dim list\" must be of the same type. This block has \"yield\" "
                    "statements of the incompatible types {} and {}.",
                    firstType->toString(), yieldType->toString()),
                yieldNode->token);
        }
    }

    // the dim list's type is thus List of firstType
    auto typeToken = firstType->token;
    statementNode->evaluatedType = boost::make_local_shared<TypeNode>(Kind::kList, typeToken, firstType);
}

static void typeCheckDimMapStatement(DimMapStatementNode* statementNode) {
    // there must be at least one yield statement
    auto& yields = *statementNode->getYieldStatementNodesList();
    if (yields.empty()) {
        throw CompilerException(
            CompilerErrorCode::kNoYieldsInDimCollection,
            "A \"dim map\" block must contain at least one \"yield\" statement.", statementNode->token);
    }

    // all yields must be of the same from/to types
    auto& firstKeyType = yields.at(0)->expression->evaluatedType;
    assert(firstKeyType != nullptr);
    auto& firstValueType = yields.at(0)->toExpression->evaluatedType;
    assert(firstValueType != nullptr);
    for (auto& yieldNode : yields) {
        auto* yieldKeyType = yieldNode->expression->evaluatedType.get();
        if (!yieldKeyType->equals(*firstKeyType)) {
            throw CompilerException(
                CompilerErrorCode::kTypeMismatch,
                fmt::format(
                    "All \"yield\" statements in a \"dim map\" block must be of the same type. This block has "
                    "\"yield\" statements with incompatible key types {} and {}.",
                    firstKeyType->toString(), yieldKeyType->toString()),
                yieldNode->token);
        }

        auto* yieldValueType = yieldNode->toExpression->evaluatedType.get();
        if (!yieldValueType->equals(*firstValueType)) {
            throw CompilerException(
                CompilerErrorCode::kTypeMismatch,
                fmt::format(
                    "All \"yield\" statements in a \"dim map\" block must be of the same type. This block has "
                    "\"yield\" statements with incompatible value types {} and {}.",
                    firstValueType->toString(), yieldValueType->toString()),
                yieldNode->token);
        }
    }

    // the dim map's type is thus Map from firstKeyType to firstValueType
    auto typeToken = firstKeyType->token;
    statementNode->evaluatedType =
        boost::make_local_shared<TypeNode>(Kind::kMap, typeToken, firstKeyType, firstValueType);
}

static void typeCheckBody(BodyNode* bodyNode, TypeCheckState* state) {
    for (auto& statementNode : bodyNode->statements) {
        statementNode->visitExpressions(true, [state](ExpressionNode* expressionNode) -> bool {
            typeCheckExpression(expressionNode, state);
            return true;
        });

        auto statementType = statementNode->getStatementType();
        switch (statementType) {
            case StatementType::kAssign:
                typeCheckAssignStatement(dynamic_cast<AssignStatementNode*>(statementNode.get()));
                break;

            case StatementType::kDim:
                typeCheckDimStatement(dynamic_cast<DimStatementNode*>(statementNode.get()));
                break;

            case StatementType::kConst:
                typeCheckConstStatement(dynamic_cast<ConstStatementNode*>(statementNode.get()));
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

            case StatementType::kForEach:
                typeCheckForEachStatement(dynamic_cast<ForEachStatementNode*>(statementNode.get()));
                break;

            case StatementType::kWhile:
                typeCheckWhileStatement(dynamic_cast<WhileStatementNode*>(statementNode.get()));
                break;

            case StatementType::kDo:
                typeCheckDoStatement(dynamic_cast<DoStatementNode*>(statementNode.get()));
                break;

            case StatementType::kThrow:
                typeCheckThrowStatement(dynamic_cast<ThrowStatementNode*>(statementNode.get()));
                break;

            case StatementType::kSelectCase:
                typeCheckSelectCaseStatement(dynamic_cast<SelectCaseStatementNode*>(statementNode.get()));
                break;

            case StatementType::kYield:
                typeCheckYieldStatement(dynamic_cast<YieldStatementNode*>(statementNode.get()));
                break;

            default:
                // do nothing
                break;
        }

        statementNode->visitBodies([state](BodyNode* innerBodyNode) -> bool {
            typeCheckBody(innerBodyNode, state);
            return true;
        });

        // dim list and dim map require the yield statements to have been type checked first
        switch (statementType) {
            case StatementType::kDimList:
                typeCheckDimListStatement(dynamic_cast<DimListStatementNode*>(statementNode.get()));
                break;

            case StatementType::kDimMap:
                typeCheckDimMapStatement(dynamic_cast<DimMapStatementNode*>(statementNode.get()));
                break;

            default:
                break;
        }
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
