#include "compileGlobals.h"
#include "CompilerException.h"
#include "parse.h"
#include "tokenize.h"
#include "typeCheck.h"
#include "util/cast.h"
#include "vm/String.h"

namespace compiler {

static boost::local_shared_ptr<TypeNode> getTypeForLiteralToken(const Token& token) {
    Kind kind = {};
    switch (token.type) {
        case TokenKind::kTrue:
        case TokenKind::kFalse:
            kind = Kind::kBoolean;
            break;
        case TokenKind::kNumberLiteral:
            kind = Kind::kNumber;
            break;
        case TokenKind::kStringLiteral:
            kind = Kind::kString;
            break;
        default:
            assert(false);
            return nullptr;
    }

    return boost::make_local_shared<TypeNode>(kind, token);
}

static boost::local_shared_ptr<vm::Object> getConstString(const LiteralStringExpressionNode& node) {
    return boost::make_local_shared<vm::String>(node.value);
}

static boost::local_shared_ptr<vm::Object> getConstObject(const ConstValueExpressionNode& node) {
    switch (node.getConstValueExpressionType()) {
        case ConstValueExpressionType::kString:
            return getConstString(dynamic_cast<const LiteralStringExpressionNode&>(node));

        default:
            assert(false);
            return nullptr;
    }
}

static vm::Value getConstValue(const ConstValueExpressionNode& node) {
    switch (node.getConstValueExpressionType()) {
        case ConstValueExpressionType::kBoolean: {
            decimal::Decimal dec{ dynamic_cast<const LiteralBooleanExpressionNode&>(node).value ? 1 : 0 };
            return vm::Value{ dec };
        }

        case ConstValueExpressionType::kNumber:
            return vm::Value{ dynamic_cast<const LiteralNumberExpressionNode&>(node).value };

        default:
            assert(false);
            return {};
    }
}

static void compileGlobal(const SourceMember& sourceMember, CompiledProgram* compiledProgram) {
    auto lowercaseIdentifier = boost::to_lower_copy(sourceMember.identifier);
    CompiledGlobalVariable* compiledGlobalVariable = nullptr;

    // see if the compiled global already exists
    for (auto& g : compiledProgram->globalVariables) {
        if (g->lowercaseName == lowercaseIdentifier) {
            throw CompilerException(
                CompilerErrorCode::kDuplicateSymbolName,
                fmt::format("The global variable name \"{}\" already exists.", sourceMember.identifier), {});
        }
    }

    // if not then create a new one
    auto g = std::make_unique<CompiledGlobalVariable>();
    g->lowercaseName = lowercaseIdentifier;
    compiledGlobalVariable = g.get();
    compiledProgram->globalVariables.push_back(std::move(g));

    auto tokens = tokenize(sourceMember.source + "\n", TokenizeType::kCompile, &sourceMember);
    auto parserResult = parse(&sourceMember, ParserRootProduction::kMember, tokens);
    if (!parserResult.isSuccess) {
        throw CompilerException(CompilerErrorCode::kSyntax, parserResult.message, *parserResult.token);
    }

    // figure out the type of the variable. it must be a literal initializer.
    if (parserResult.node->getMemberType() == MemberType::kConstStatement) {
        auto& constNode = dynamic_cast<ConstStatementNode&>(*parserResult.node);
        parserResult.node->evaluatedType = getTypeForLiteralToken(constNode.value->token);
    } else if (parserResult.node->getMemberType() == MemberType::kDimStatement) {
        const auto* dimNode = util::dynamic_cast_borrow<DimStatementNode>(parserResult.node);
        assert(dimNode->value || dimNode->type);
        if (dimNode->value) {
            if (dimNode->value->getExpressionType() != ExpressionType::kConstValue) {
                throw CompilerException(
                    CompilerErrorCode::kInvalidGlobalVariableType,
                    "Global variable initial values must be a boolean, number, or string literal.",
                    dimNode->value->token);
            }
            const auto* constValueNode = util::dynamic_cast_borrow<ConstValueExpressionNode>(dimNode->value);
            parserResult.node->evaluatedType = getTypeForLiteralToken(constValueNode->token);
        } else {
            parserResult.node->evaluatedType = boost::make_local_shared<TypeNode>(*dimNode->type);
        }
    } else {
        throw CompilerException(
            CompilerErrorCode::kWrongMemberType,
            "This member must be a global variable (dim) or constant value (const).", tokens[0]);
    }

    compiledGlobalVariable->isValue = parserResult.node->evaluatedType->isValueType();
    vm::Value initialValue{ decimal::Decimal{ 0 } };
    boost::local_shared_ptr<vm::Object> initialObject{};

    ExpressionNode* valueExpr = nullptr;
    if (parserResult.node->getMemberType() == MemberType::kDimStatement) {
        auto& dimNode = dynamic_cast<DimStatementNode&>(*parserResult.node);
        valueExpr = dimNode.value.get();
    }
    if (parserResult.node->getMemberType() == MemberType::kConstStatement) {
        auto& constNode = dynamic_cast<ConstStatementNode&>(*parserResult.node);
        valueExpr = constNode.value.get();
    }
    if (valueExpr) {
        if (valueExpr->getExpressionType() != ExpressionType::kConstValue) {
            throw CompilerException(
                CompilerErrorCode::kInvalidGlobalVariableType,
                "The initial value of a global variable must be a boolean, number, or string literal.",
                valueExpr->token);
        }
        auto& constValueExpr = dynamic_cast<ConstValueExpressionNode&>(*valueExpr);
        if (compiledGlobalVariable->isValue) {
            initialValue = getConstValue(constValueExpr);
        } else {
            initialObject = getConstObject(constValueExpr);
        }
    }

    if (compiledGlobalVariable->isValue) {
        compiledGlobalVariable->index = compiledProgram->vmProgram.globalValues.size();
        compiledProgram->vmProgram.globalValues.push_back(initialValue);
    } else {
        compiledGlobalVariable->index = compiledProgram->vmProgram.globalObjects.size();
        compiledProgram->vmProgram.globalObjects.push_back(std::move(initialObject));
    }

    compiledGlobalVariable->dimOrConstStatementNode = std::move(parserResult.node);
}

void compileGlobals(const SourceProgram& sourceProgram, CompiledProgram* compiledProgram) {
    for (const auto& member : sourceProgram.members) {
        if (member->memberType == SourceMemberType::kGlobal) {
            compileGlobal(*member, compiledProgram);
        }
    }
}

}  // namespace compiler
