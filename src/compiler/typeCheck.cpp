#include "typeCheck.h"
#include "CompilerException.h"
#include "vm/systemCall.h"

namespace compiler {

class BuiltInProcedureList {
   private:
    std::vector<std::unique_ptr<ProcedureNode>> _empty{};
    std::unordered_map<std::string, std::unique_ptr<std::vector<std::unique_ptr<ProcedureNode>>>> _map{};

    std::vector<std::unique_ptr<ProcedureNode>>* findOrCreateVector(const std::string& lowercaseName) {
        auto it = _map.find(lowercaseName);
        if (it != _map.end()) {
            return it->second.get();
        }

        auto procedureNode = std::make_unique<std::vector<std::unique_ptr<ProcedureNode>>>();
        auto procedureNodePtr = procedureNode.get();
        _map.insert(std::pair(lowercaseName, std::move(procedureNode)));
        return procedureNodePtr;
    }

    ProcedureNode* addSub(
        std::string name,
        std::initializer_list<std::string> parameterNames,
        std::initializer_list<boost::local_shared_ptr<TypeNode>> parameterTypes,
        vm::SystemCall systemCall) {
        auto lowercaseName = boost::to_lower_copy(name);
        assert(parameterNames.size() == parameterTypes.size());
        std::vector<std::unique_ptr<ParameterNode>> parameterNodes{};
        auto* parameterNameIter = parameterNames.begin();
        auto* parameterTypesIter = parameterTypes.begin();
        for (size_t i = 0; i < parameterNames.size(); i++) {
            parameterNodes.push_back(std::make_unique<ParameterNode>(
                std::move(*parameterNameIter++), std::move(*parameterTypesIter++), Token{}));
        }
        auto procedure = std::make_unique<ProcedureNode>(std::move(name), std::move(parameterNodes), nullptr, Token{});
        procedure->systemCall = systemCall;
        auto* procedurePtr = procedure.get();
        auto* vec = findOrCreateVector(lowercaseName);
        vec->push_back(std::move(procedure));
        return procedurePtr;
    }

    ProcedureNode* addFunction(
        std::string name,
        std::initializer_list<std::string> parameterNames,
        std::initializer_list<boost::local_shared_ptr<TypeNode>> parameterTypes,
        boost::local_shared_ptr<TypeNode> returnType,
        vm::SystemCall systemCall) {
        auto* node = addSub(name, parameterNames, parameterTypes, systemCall);
        node->returnType = std::move(returnType);
        return node;
    }

   public:
    BuiltInProcedureList() {
        auto number = boost::make_local_shared<TypeNode>(Kind::kNumber, Token{});
        auto string = boost::make_local_shared<TypeNode>(Kind::kString, Token{});

        addFunction("Chr", { "input" }, { number }, string, vm::SystemCall::kChr);
    }

    const std::vector<std::unique_ptr<ProcedureNode>>& get(const std::string& name) {
        auto lowercaseName = boost::to_lower_copy(name);
        auto result = _map.find(lowercaseName);
        return result == _map.end() ? _empty : *result->second;
    }
};

class TypeCheckState {
   public:
    const SourceProgram& sourceProgram;
    CompiledProgram* compiledProgram;
    BuiltInProcedureList builtInProcedures{};
    TypeCheckState(const SourceProgram& sourceProgram, CompiledProgram* compiledProgram)
        : sourceProgram(sourceProgram), compiledProgram(compiledProgram) {}
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
    for (auto& argument : arguments) {
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

    for (auto& builtInProcedure : state->builtInProcedures.get(name)) {
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

static void typeCheckCallExpression(CallExpressionNode* expressionNode, TypeCheckState* state) {
    typeCheckCall(expressionNode, expressionNode->name, expressionNode->arguments, state, true);
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
            typeCheckSymbolReferenceExpression(dynamic_cast<SymbolReferenceExpressionNode*>(expressionNode), state);
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

static void typeCheckCallStatement(CallStatementNode* statementNode, TypeCheckState* state) {
    typeCheckCall(statementNode, statementNode->name, statementNode->arguments, state, false);
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

            default:
                // do nothing
                break;
        }
    }
}

void typeCheck(ProcedureNode* procedureNode, const SourceProgram& sourceProgram, CompiledProgram* compiledProgram) {
    TypeCheckState state{ sourceProgram, compiledProgram };
    return typeCheckBody(procedureNode->body.get(), &state);
}

};  // namespace compiler
