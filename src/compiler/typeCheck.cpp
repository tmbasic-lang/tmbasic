#include "typeCheck.h"
#include "BuiltInProcedureList.h"
#include "CompilerException.h"
#include "vm/systemCall.h"

namespace compiler {

class TypeCheckState {
   public:
    const ProcedureNode& procedureNode;
    const SourceProgram& sourceProgram;
    CompiledProgram* compiledProgram;
    const BuiltInProcedureList& builtInProcedures;
    boost::local_shared_ptr<TypeNode> typeBoolean{ boost::make_local_shared<TypeNode>(Kind::kBoolean, Token{}) };
    boost::local_shared_ptr<TypeNode> typeNumber{ boost::make_local_shared<TypeNode>(Kind::kNumber, Token{}) };
    TypeCheckState(
        const ProcedureNode& procedureNode,
        const SourceProgram& sourceProgram,
        CompiledProgram* compiledProgram,
        const BuiltInProcedureList& builtInProcedures)
        : procedureNode(procedureNode),
          sourceProgram(sourceProgram),
          compiledProgram(compiledProgram),
          builtInProcedures(builtInProcedures) {}
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

        if (parameterType->kind == Kind::kGeneric1 || parameterType->kind == Kind::kGeneric2) {
            // Substitute the appropriate generic type from the first argument of this function.
            // For example, ContainsKey(map as Map from Any to Any, key as Generic1) as Boolean
            const auto& firstArgumentType = arguments.at(0)->evaluatedType;
            switch (firstArgumentType->kind) {
                case Kind::kList:
                    if (parameterType->kind == Kind::kGeneric1) {
                        parameterType = firstArgumentType->listItemType;
                    } else {
                        throw CompilerException(
                            CompilerErrorCode::kInternal,
                            "Internal error. This built-in procedure has a Generic2 parameter type but its first "
                            "parameter is a List.",
                            Token{});
                    }
                    break;

                case Kind::kMap:
                    if (parameterType->kind == Kind::kGeneric1) {
                        parameterType = firstArgumentType->mapKeyType;
                    } else {
                        parameterType = firstArgumentType->mapValueType;
                    }
                    break;

                default:
                    throw CompilerException(
                        CompilerErrorCode::kInternal,
                        "Internal error. This built-in procedure has a Generic1 or Generic2 parameter type but the "
                        "first parameter type is not supported.",
                        Token{});
            }
        }

        if (!parameterType->isImplicitlyAssignableFrom(*argumentType)) {
            return false;
        }
    }

    return true;
}

static bool doCallArgumentTypesMatchProcedureParameters(
    const std::vector<std::unique_ptr<TypeNode>>& arguments,
    const std::vector<std::unique_ptr<ParameterNode>>& parameters) {
    auto parameterCount = parameters.size();
    auto argumentCount = arguments.size();
    if (parameterCount != argumentCount) {
        return false;
    }

    for (size_t i = 0; i < parameterCount; i++) {
        auto& parameterType = parameters.at(i)->type;
        assert(parameterType != nullptr);
        const auto& argumentType = arguments.at(i);
        assert(argumentType != nullptr);

        if (!parameterType->isImplicitlyAssignableFrom(*argumentType)) {
            return false;
        }
    }

    return true;
}

static void replaceImplicitArgumentTypeConversionsWithExplicit(
    std::vector<std::unique_ptr<ExpressionNode>>* arguments,
    const std::vector<std::unique_ptr<ParameterNode>>& parameters) {
    // MARKER: This function concerns implicit type conversions. Search for this line to find others.
    // We assume that doCallArgumentTypesMatchProcedureParameters() was already called and returned true.
    for (size_t i = 0; i < parameters.size(); i++) {
        auto& parameterType = parameters.at(i)->type;
        auto& argumentType = *arguments->at(i)->evaluatedType;
        if (parameterType->equals(argumentType)) {
            continue;  // No implicit type conversion.
        }

        // Copy the token first, before we move from arguments[i]
        auto token = arguments->at(i)->token;

        // Concrete->Generic must not be converted because a ConvertExpressionNode can't convert to a generic type.
        // This isn't just for performance; these early continues are required for correctness.
        if (parameterType->kind == Kind::kAny) {
            continue;
        }
        if (parameterType->kind == Kind::kList && argumentType.kind == Kind::kList &&
            parameterType->listItemType->kind == Kind::kAny) {
            continue;
        }
        if (parameterType->kind == Kind::kMap && argumentType.kind == Kind::kMap &&
            parameterType->mapKeyType->kind == Kind::kAny && parameterType->mapValueType->kind == Kind::kAny) {
            continue;
        }
        if (parameterType->kind == Kind::kOptional && argumentType.kind == Kind::kOptional &&
            parameterType->optionalValueType->kind == Kind::kAny) {
            continue;
        }

        // Any other conversion requires us to insert an "X as Y" convert expression.
        // After the type check phase ends, all implicit conversions have been replaced with explicit ones.
        auto convertExpression =
            std::make_unique<ConvertExpressionNode>(std::move(arguments->at(i)), parameterType, token);
        convertExpression->evaluatedType = parameterType;
        arguments->at(i) = std::move(convertExpression);
    }
}

static void typeCheckCall(
    Node* callNode,
    const std::string& name,
    std::vector<std::unique_ptr<ExpressionNode>>* arguments,
    TypeCheckState* state,
    bool mustBeFunction) {
    for (const auto& argument : *arguments) {
        typeCheckExpression(argument.get(), state);
    }

    auto anyNameMatches = false;

    // MARKER: This function concerns procedure lookups. Search for this line to find others.

    auto lowercaseProcedureName = boost::to_lower_copy(name);
    for (auto& compiledProcedure : state->compiledProgram->procedures) {
        if (compiledProcedure->nameLowercase == lowercaseProcedureName) {
            anyNameMatches = true;
            if (doCallArgumentTypesMatchProcedureParameters(*arguments, compiledProcedure->procedureNode->parameters)) {
                replaceImplicitArgumentTypeConversionsWithExplicit(
                    arguments, compiledProcedure->procedureNode->parameters);
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
        if (doCallArgumentTypesMatchProcedureParameters(*arguments, builtInProcedure->parameters)) {
            replaceImplicitArgumentTypeConversionsWithExplicit(arguments, builtInProcedure->parameters);
            if (mustBeFunction && builtInProcedure->returnType == nullptr) {
                throw CompilerException(
                    CompilerErrorCode::kSubCalledAsFunction,
                    fmt::format("\"{}\" is a subroutine but is being called as a function.", name), callNode->token);
            }
            callNode->systemCall = builtInProcedure->systemCall;
            if (builtInProcedure->returnType != nullptr) {
                // If the return type is Generic1/2 then pick from the first argument's generic type(s).
                if (builtInProcedure->returnType->isGeneric()) {
                    if (arguments->empty()) {
                        throw CompilerException(
                            CompilerErrorCode::kInternal,
                            "Internal error. Built-in procedure has generic return type but no arguments.",
                            callNode->token);
                    }

                    boost::local_shared_ptr<TypeNode> concrete1{};
                    boost::local_shared_ptr<TypeNode> concrete2{};

                    auto& argType = arguments->at(0)->evaluatedType;
                    assert(argType != nullptr);
                    switch (argType->kind) {
                        case Kind::kList:
                            concrete1 = argType->listItemType;
                            assert(concrete1 != nullptr);
                            break;
                        case Kind::kMap:
                            concrete1 = argType->mapKeyType;
                            concrete2 = argType->mapValueType;
                            assert(concrete1 != nullptr);
                            assert(concrete2 != nullptr);
                            break;
                        case Kind::kOptional:
                            concrete1 = argType->optionalValueType;
                            assert(concrete1 != nullptr);
                            break;
                        default:
                            concrete1 = argType;
                            break;
                    }

                    switch (builtInProcedure->returnType->kind) {
                        case Kind::kAny:
                            throw CompilerException(
                                CompilerErrorCode::kInternal,
                                "Internal error. Built-in procedure has return type of \"any\" which is not allowed.",
                                callNode->token);

                        case Kind::kGeneric1:
                            if (concrete1 == nullptr) {
                                throw CompilerException(
                                    CompilerErrorCode::kInternal,
                                    "Internal error. Built-in procedure has return type of \"Generic1\" but the first "
                                    "parameter isn't a generic type.",
                                    callNode->token);
                            }
                            callNode->evaluatedType = std::move(concrete1);
                            break;

                        case Kind::kGeneric2:
                            if (concrete1 == nullptr || concrete2 == nullptr) {
                                throw CompilerException(
                                    CompilerErrorCode::kInternal,
                                    "Internal error. Built-in procedure has return type of \"Generic2\" but the first "
                                    "parameter isn't a Map.",
                                    callNode->token);
                            }
                            callNode->evaluatedType = std::move(concrete2);
                            break;

                        case Kind::kList: {
                            auto listItemTypeKind = builtInProcedure->returnType->listItemType->kind;
                            if (listItemTypeKind == Kind::kGeneric1) {
                                callNode->evaluatedType =
                                    boost::make_local_shared<TypeNode>(Kind::kList, Token{}, std::move(concrete1));
                            } else {
                                throw CompilerException(
                                    CompilerErrorCode::kInternal,
                                    "Internal error. Built-in procedure has return type of \"List\" with an invalid "
                                    "type parameter.",
                                    callNode->token);
                            }
                            break;
                        }

                        case Kind::kOptional: {
                            auto optionalValueTypeKind = builtInProcedure->returnType->optionalValueType->kind;
                            if (optionalValueTypeKind == Kind::kGeneric1) {
                                callNode->evaluatedType =
                                    boost::make_local_shared<TypeNode>(Kind::kOptional, Token{}, std::move(concrete1));
                            } else if (optionalValueTypeKind == Kind::kGeneric2) {
                                callNode->evaluatedType =
                                    boost::make_local_shared<TypeNode>(Kind::kOptional, Token{}, std::move(concrete2));
                            } else {
                                throw CompilerException(
                                    CompilerErrorCode::kInternal,
                                    "Internal error. Built-in procedure has return type of \"Optional\" with an "
                                    "invalid type parameter.",
                                    callNode->token);
                            }
                            break;
                        }

                        default:
                            throw CompilerException(
                                CompilerErrorCode::kInternal,
                                fmt::format(
                                    "Internal error. Built-in procedure has return type of {} which is not "
                                    "implemented.",
                                    builtInProcedure->returnType->toString()),
                                callNode->token);
                    }

                } else {
                    callNode->evaluatedType = builtInProcedure->returnType;
                }

                assert(callNode->evaluatedType != nullptr);
                assert(!callNode->evaluatedType->isGeneric());
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

static void typeCheckConvertExpression(ConvertExpressionNode* expressionNode, TypeCheckState* state) {
    typeCheckExpression(expressionNode->value.get(), state);

    assert(expressionNode->value->evaluatedType != nullptr);
    assert(expressionNode->type != nullptr);

    const auto& srcType = *expressionNode->value->evaluatedType;
    const auto& dstType = *expressionNode->type;

    // Named type <-> anonymous record type
    if (srcType.kind == Kind::kRecord && dstType.kind == Kind::kRecord) {
        if (srcType.fields.size() != dstType.fields.size()) {
            throw CompilerException(
                CompilerErrorCode::kInvalidTypeConversion,
                fmt::format(
                    "This type conversion is not allowed because the target type has {} field(s) but the source type "
                    "has {} field(s). The number of fields must match.",
                    dstType.fields.size(), srcType.fields.size()),
                expressionNode->token);
        }

        auto numFields = srcType.fields.size();
        for (size_t i = 0; i < numFields; i++) {
            auto& srcField = *srcType.fields.at(i);
            auto& dstField = *dstType.fields.at(i);

            if (srcField.nameLowercase != dstField.nameLowercase) {
                throw CompilerException(
                    CompilerErrorCode::kInvalidTypeConversion,
                    fmt::format(
                        "This type conversion is not allowed because the source and target types have different field "
                        "names. The field at index {} is named \"{}\" in the source type but \"{}\" in the target "
                        "type.",
                        i, srcField.name, dstField.name),
                    expressionNode->token);
            }

            if (!srcField.type->equals(*dstField.type)) {
                throw CompilerException(
                    CompilerErrorCode::kInvalidTypeConversion,
                    fmt::format(
                        "This type conversion is not allowed because the source and target type have different field "
                        "types. The field \"{}\" has type \"{}\" in the source type but type \"{}\" in the target "
                        "type.",
                        srcField.name, srcField.type->toString(), dstField.type->toString()),
                    expressionNode->token);
            }
        }

        // The record types are compatible.
        expressionNode->evaluatedType = expressionNode->type;
        return;
    }

    // Number -> String, Date -> DateTime, DateTime -> Date, DateTimeOffset -> Date, DateTimeOffset -> DateTime
    if ((srcType.kind == Kind::kNumber && dstType.kind == Kind::kString) ||
        (srcType.kind == Kind::kDate && dstType.kind == Kind::kDateTime) ||
        (srcType.kind == Kind::kDateTime && dstType.kind == Kind::kDate) ||
        (srcType.kind == Kind::kDateTimeOffset && dstType.kind == Kind::kDate) ||
        (srcType.kind == Kind::kDateTimeOffset && dstType.kind == Kind::kDateTime)) {
        expressionNode->evaluatedType = expressionNode->type;
        return;
    }

    // T -> Optional T
    if (dstType.kind == Kind::kOptional && dstType.optionalValueType->equals(srcType)) {
        expressionNode->evaluatedType = expressionNode->type;
        return;
    }

    throw CompilerException(
        CompilerErrorCode::kInvalidTypeConversion, "This type conversion is not allowed.", expressionNode->token);
}

static void typeCheckFunctionCallExpression(FunctionCallExpressionNode* expressionNode, TypeCheckState* state) {
    typeCheckCall(expressionNode, expressionNode->name, &expressionNode->args, state, true);
}

static void typeCheckDottedExpression(DottedExpressionNode* expressionNode, TypeCheckState* state) {
    typeCheckExpression(expressionNode->base.get(), state);
    auto baseType = expressionNode->base->evaluatedType;

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
                if (baseType->fields.empty()) {
                    throw CompilerException(
                        CompilerErrorCode::kFieldNotFound,
                        fmt::format(
                            "The field \"{}\" does not exist. The type \"{}\" does not have any fields.",
                            *usageSuffix->name, baseType->toString()),
                        usageSuffix->token);
                }
                std::string fields;
                for (const auto& field : baseType->fields) {
                    fields += field->name + ", ";
                }
                fields = fields.substr(0, fields.size() - 2);  // Remove the trailing comma and space
                throw CompilerException(
                    CompilerErrorCode::kFieldNotFound,
                    fmt::format(
                        "The type \"{}\" does not have a field named \"{}\". Available fields: {}.",
                        baseType->toString(), *usageSuffix->name, fields),
                    usageSuffix->token);
            }

            assert(typeField->fieldValueIndex.has_value() || typeField->fieldObjectIndex.has_value());
            usageSuffix->boundParameterNode = typeField;
            baseType = typeField->type;
            usageSuffix->evaluatedType = baseType;
        } else if (usageSuffix->isIndex()) {
            if (usageSuffix->collectionIndex.size() != 1) {
                throw CompilerException(
                    CompilerErrorCode::kInvalidListIndex,
                    fmt::format(
                        "The parentheses here look like a List or Map index, but there are {} comma-separated "
                        "expressions inside these parentheses instead of 1.",
                        usageSuffix->collectionIndex.size()),
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

            auto& collectionIndex = usageSuffix->collectionIndex.at(0);

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

            usageSuffix->evaluatedType = baseType;
        } else {
            assert(false);
        }
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

static void typeCheckSymbolReferenceExpression(SymbolReferenceExpressionNode* expressionNode) {
    const auto* decl = expressionNode->boundSymbolDeclaration;
    assert(decl != nullptr);
    assert(decl->getSymbolDeclaration().has_value());

    // We don't support calling a parameterless function with no parentheses.
    // Let's just double-check that the binder didn't try to do it anyway.
    assert(dynamic_cast<const ProcedureNode*>(decl) == nullptr);

    // It must be a regular variable declaration
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
            typeCheckConvertExpression(dynamic_cast<ConvertExpressionNode*>(expressionNode), state);
            break;
        case ExpressionType::kDotted:
            typeCheckDottedExpression(dynamic_cast<DottedExpressionNode*>(expressionNode), state);
            break;
        case ExpressionType::kNot:
            typeCheckNotExpression(dynamic_cast<NotExpressionNode*>(expressionNode), state);
            break;
        case ExpressionType::kSymbolReference:
            typeCheckSymbolReferenceExpression(dynamic_cast<SymbolReferenceExpressionNode*>(expressionNode));
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
    // MARKER: This function concerns implicit type conversions. Search for this line to find others.

    typeCheckAssignToExpression(*statementNode->target);

    // RHS type must be implicitly convertible to LHS type.
    assert(statementNode->target->evaluatedType != nullptr);
    auto& lhsType = *statementNode->target->evaluatedType;
    assert(statementNode->value->evaluatedType != nullptr);
    auto& rhsType = *statementNode->value->evaluatedType;
    if (!lhsType.isImplicitlyAssignableFrom(rhsType)) {
        throw CompilerException(
            CompilerErrorCode::kTypeMismatch,
            fmt::format(
                "The types on both sides of an assignment must match, but these types are {} and {}.",
                lhsType.toString(), rhsType.toString()),
            statementNode->token);
    }

    // If the types aren't exactly identical, then turn the implicit conversion into an explicit one.
    // We don't have to worry about concrete->generic here; both LHS and RHS will be concrete types.
    if (!lhsType.equals(rhsType)) {
        auto token = statementNode->value->token;
        auto convertExpression = std::make_unique<ConvertExpressionNode>(
            std::move(statementNode->value), statementNode->target->evaluatedType, token);
        convertExpression->evaluatedType = statementNode->target->evaluatedType;
        statementNode->value = std::move(convertExpression);
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
    typeCheckCall(statementNode, statementNode->name, &statementNode->arguments, state, false);
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
    assert(statementNode->boundCollectionDeclaration != nullptr);
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

static void typeCheckOnStatement(OnStatementNode* statementNode, TypeCheckState* state) {
    assert(statementNode->target->evaluatedType != nullptr);
    auto targetKind = statementNode->target->evaluatedType->kind;

    // Validate the event type and come up with the parameter list for the event handler.
    std::vector<std::unique_ptr<TypeNode>> expectedHandlerParameters;
    if (targetKind == Kind::kForm) {
        if (statementNode->eventName == "shown") {
            expectedHandlerParameters.push_back(std::make_unique<TypeNode>(Kind::kForm, Token{}));
        } else {
            throw CompilerException(
                CompilerErrorCode::kTypeMismatch,
                fmt::format("The event name \"{}\" is not valid for forms.", statementNode->eventName),
                statementNode->token);
        }
    } else if (targetKind == Kind::kControl) {
        throw std::runtime_error("not impl");
    } else {
        throw CompilerException(
            CompilerErrorCode::kTypeMismatch,
            fmt::format(
                "The \"on\" statement target must be a Form or Control, but this target type is {}.",
                statementNode->target->evaluatedType->toString()),
            statementNode->token);
    }

    // Find the specified event handler procedure with the expected parameters.
    // MARKER: This function concerns procedure lookups. Search for this line to find others.
    auto lowercaseProcedureName = boost::to_lower_copy(statementNode->handlerName);
    auto nameMatch = false;
    auto signatureMatch = false;
    for (auto& compiledProcedure : state->compiledProgram->procedures) {
        if (compiledProcedure->nameLowercase == lowercaseProcedureName) {
            nameMatch = true;
            if (doCallArgumentTypesMatchProcedureParameters(
                    expectedHandlerParameters, compiledProcedure->procedureNode->parameters)) {
                signatureMatch = true;
                statementNode->procedureIndex = compiledProcedure->procedureIndex;
            }
        }
    }

    if (!nameMatch) {
        throw CompilerException(
            CompilerErrorCode::kProcedureNotFound,
            fmt::format("The event handler procedure \"{}\" was not found.", statementNode->handlerName),
            statementNode->token);
    }

    if (!signatureMatch) {
        throw CompilerException(
            CompilerErrorCode::kTypeMismatch,
            fmt::format(
                "The event handler procedure \"{}\" does not accept the right parameters for this event.",
                statementNode->handlerName),
            statementNode->token);
    }
}

static void typeCheckReturnStatement(ReturnStatementNode* statementNode, TypeCheckState* state) {
    // The return expression has been type checked, but we need to ensure it matches the procedure's return type.
    const auto& expectedType = state->procedureNode.returnType;

    if (statementNode->expression == nullptr) {
        // This is a bare "return" statement which must be inside a subroutine.
        if (expectedType != nullptr) {
            throw CompilerException(
                CompilerErrorCode::kInvalidReturn,
                fmt::format("This function expects to return a value of type {}.", expectedType->toString()),
                statementNode->token);
        }
    } else {
        // This is a "return x" statement which must be inside a function.
        if (expectedType == nullptr) {
            throw CompilerException(
                CompilerErrorCode::kInvalidReturn, "A subroutine cannot return a value.", statementNode->token);
        }

        const auto* actualType = statementNode->expression->evaluatedType.get();
        assert(actualType != nullptr);

        if (!expectedType->isImplicitlyAssignableFrom(*actualType)) {
            throw CompilerException(
                CompilerErrorCode::kTypeMismatch,
                fmt::format(
                    "The return type of this function is {}, but the \"return\" statement expression is of type {}.",
                    expectedType->toString(), actualType->toString()),
                statementNode->token);
        }

        // MARKER: This function concerns implicit type conversions. Search for this line to find others.
        if (!expectedType->equals(*actualType)) {
            // actualType and expectedType aren't equal but an implicit conversion exists.
            // Convert the implicit conversion to an explicit conversion.
            // After the type check phase ends, all implicit conversions have been replaced with explicit ones.
            auto token = statementNode->expression->token;
            auto convertExpression =
                std::make_unique<ConvertExpressionNode>(std::move(statementNode->expression), expectedType, token);
            convertExpression->evaluatedType = expectedType;
            statementNode->expression = std::move(convertExpression);
        }
    }
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

            case StatementType::kOn:
                typeCheckOnStatement(dynamic_cast<OnStatementNode*>(statementNode.get()), state);
                break;

            case StatementType::kReturn:
                typeCheckReturnStatement(dynamic_cast<ReturnStatementNode*>(statementNode.get()), state);
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
    TypeCheckState state{ *procedureNode, sourceProgram, compiledProgram, builtInProcedures };
    return typeCheckBody(procedureNode->body.get(), &state);
}

};  // namespace compiler
