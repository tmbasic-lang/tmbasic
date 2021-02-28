#include "compiler/compile.h"
#include "compiler/bindProcedureSymbols.h"
#include "compiler/parse.h"
#include "compiler/tokenize.h"
#include "compiler/typeCheck.h"
#include "util/cast.h"

using util::dynamic_cast_move;

namespace compiler {

static std::unique_ptr<TypeNode> getTypeForLiteralToken(const Token& token) {
    Kind kind = {};
    switch (token.type) {
        case TokenKind::kBooleanLiteral:
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

    return std::make_unique<TypeNode>(kind, token);
}

static int getFirstAvailableGlobalVariableIndex(const CompiledProgram& compiledProgram) {
    int maxIndex = -1;
    for (const auto& g : compiledProgram.globalVariables) {
        if (g->index > maxIndex) {
            maxIndex = g->index;
        }
    }

    auto inUse = std::vector<bool>(maxIndex + 1, false);
    for (const auto& g : compiledProgram.globalVariables) {
        // the variable we are trying to allocate will have -1 for its index
        if (g->index != -1) {
            inUse.at(g->index) = true;
        }
    }

    for (auto i = 0; i <= maxIndex; i++) {
        if (!inUse.at(i)) {
            return i;
        }
    }

    return maxIndex + 1;
}

static CompilerResult compileGlobal(const SourceMember& sourceMember, CompiledProgram* compiledProgram) {
    auto lowercaseIdentifier = boost::algorithm::to_lower_copy(sourceMember.identifier);
    CompiledGlobalVariable* compiledGlobalVariable = nullptr;

    // see if the compiled global already exists
    for (auto& g : compiledProgram->globalVariables) {
        if (g->lowercaseName == lowercaseIdentifier) {
            compiledGlobalVariable = g.get();
        }
    }

    // if not then create a new one
    if (compiledGlobalVariable == nullptr) {
        auto g = std::make_unique<CompiledGlobalVariable>();
        g->lowercaseName = lowercaseIdentifier;
        compiledGlobalVariable = g.get();
        compiledProgram->globalVariables.push_back(std::move(g));
    }

    auto tokens = tokenize(sourceMember.source, TokenizeType::kCompile);
    auto parserResult = parse(ParserRootProduction::kMember, tokens);
    if (!parserResult.isSuccess) {
        return CompilerResult::error(parserResult.message, *parserResult.token);
    }

    // figure out the type of the variable. it must be a literal initializer.
    if (parserResult.node->getMemberType() == MemberType::kConstStatement) {
        const auto* constNode = util::dynamic_cast_borrow<ConstStatementNode>(parserResult.node);
        compiledGlobalVariable->type = getTypeForLiteralToken(constNode->value->token);
        assert(compiledGlobalVariable->type);
    } else if (parserResult.node->getMemberType() == MemberType::kDimStatement) {
        const auto* dimNode = util::dynamic_cast_borrow<DimStatementNode>(parserResult.node);
        assert(dimNode->value || dimNode->type);
        if (dimNode->value) {
            if (dimNode->value->getExpressionType() != ExpressionType::kConstValue) {
                return CompilerResult::error(
                    "Global variable initial values must be a boolean, number, or string literal.",
                    dimNode->value->token);
            }
            const auto* constValueNode = util::dynamic_cast_borrow<ConstValueExpressionNode>(dimNode->value);
            compiledGlobalVariable->type = getTypeForLiteralToken(constValueNode->token);
        } else {
            compiledGlobalVariable->type = std::make_unique<TypeNode>(*dimNode->type);
        }
    } else {
        return CompilerResult::error(
            "This member must be a global variable (dim) or constant value (const).", tokens[0]);
    }

    compiledGlobalVariable->isValue = compiledGlobalVariable->type->isValueType();
    compiledGlobalVariable->index = -1;  // it may already be set, so unset it before allocating an index
    compiledGlobalVariable->index = getFirstAvailableGlobalVariableIndex(*compiledProgram);

    return CompilerResult::success();
}

// precondition: all global variables must be compiled first
static CompilerResult compileProcedure(const SourceMember& sourceMember, CompiledProgram* compiledProgram) {
    auto tokens = tokenize(sourceMember.source, TokenizeType::kCompile);
    auto parserResult = parse(ParserRootProduction::kMember, tokens);
    if (!parserResult.isSuccess) {
        return CompilerResult::error(parserResult.message, *parserResult.token);
    }
    if (parserResult.node->getMemberType() != MemberType::kProcedure) {
        return CompilerResult::error("This member must be a subroutine or function.", tokens[0]);
    }
    auto procedureNode = dynamic_cast_move<ProcedureNode>(std::move(parserResult.node));

    auto compilerResult = bindProcedureSymbols(procedureNode.get(), *compiledProgram);
    if (!compilerResult.isSuccess) {
        return compilerResult;
    }

    compilerResult = typeCheck(procedureNode.get());
    if (!compilerResult.isSuccess) {
        return compilerResult;
    }

    return CompilerResult::success();
}

CompilerResult compile(const SourceMember& sourceMember, CompiledProgram* compiledProgram) {
    switch (sourceMember.memberType) {
        case SourceMemberType::kProcedure:
            return compileProcedure(sourceMember, compiledProgram);
        case SourceMemberType::kGlobal:
            return compileGlobal(sourceMember, compiledProgram);
        default:
            assert(false);
            throw std::runtime_error("Not implemented");
    }
}

}  // namespace compiler
