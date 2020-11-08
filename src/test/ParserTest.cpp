#include "common.h"
#include "gtest/gtest.h"
#include "core/basic/Ast.h"
#include "core/compiler/Parser.h"
#include "core/compiler/Scanner.h"
#include "core/util/cast.h"

using namespace basic;
using namespace compiler;

static std::string dump(Node& node) {
    std::ostringstream s;
    node.dump(s, 0);
    return s.str();
}

static std::string readFile(std::string filename) {
    std::ifstream stream("/code/src/test/programs/" + filename);
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

static void parseMatch(std::string filenameWithoutExtension) {
    Parser parser;
    auto ast = readFile(filenameWithoutExtension + ".ast");
    auto bas = readFile(filenameWithoutExtension + ".bas");
    auto tokens = Scanner::tokenize(bas);
    auto result = parser.parseProgram(tokens);
    if (!result.isSuccess) {
        std::cout << "Token: " << result.token.value().text << std::endl;
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
