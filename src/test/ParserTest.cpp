#include "../common.h"
#include "compiler/parse.h"
#include "compiler/tokenize.h"
#include "gtest/gtest.h"
#include "helpers.h"
#include "shared/basic/ast.h"
#include "shared/util/cast.h"

using basic::Node;
using basic::ProgramNode;
using basic::Token;
using basic::TokenKind;
using compiler::parse;
using compiler::ParserRootProduction;
using compiler::tokenize;
using util::dynamic_cast_borrow;

static std::string dump(const Node& node) {
    std::ostringstream s;
    node.dump(s, 0);
    return s.str();
}

static void parseMatch(std::string filenameWithoutExtension) {
    auto ast = readFile(filenameWithoutExtension + ".ast");
    auto bas = readFile(filenameWithoutExtension + ".bas");
    auto tokens = tokenize(bas);
    auto result = parse(ParserRootProduction::kProgram, tokens);
    if (!result.isSuccess) {
        std::cout << "Token: " << (*result.token).text << std::endl;
        std::cout << "Message: " << result.message << std::endl;
    }
    ASSERT_TRUE(result.isSuccess);
    std::ostringstream s;
    dynamic_cast_borrow<ProgramNode>(result.node)->dump(s, 0);
    if (ast != s.str()) {
        std::cout << "Actual:\n" << s.str() << std::endl;
    }
    ASSERT_EQ(ast, s.str());
}

TEST(ParserTest, SingleProgramNodeDump) {
    auto n = ProgramNode(std::vector<std::unique_ptr<Node>>(), Token(0, 0, TokenKind::kEndOfFile, ""));
    ASSERT_EQ("ProgramNode\n", dump(n));
}

TEST(ParserTest, ParseEmpty) {
    parseMatch("Empty");
}

TEST(ParserTest, ParseSimpleSub) {
    parseMatch("SimpleSub");
}

TEST(ParserTest, ParseSimpleFunction) {
    parseMatch("SimpleFunction");
}
