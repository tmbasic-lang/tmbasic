#include "../common.h"
#include "compiler/tokenize.h"
#include "gtest/gtest.h"
#include "shared/basic/TokenKind.h"

using basic::TokenKind;
using compiler::tokenize;

static std::string readFile(std::string filename) {
    std::ifstream stream("/code/src/test/programs/" + filename);
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

static void scanMatch(std::string filenameWithoutExtension) {
    auto tok = readFile(filenameWithoutExtension + ".tok");
    auto bas = readFile(filenameWithoutExtension + ".bas");
    auto tokens = tokenize(bas);
    std::ostringstream s;
    for (auto token : tokens) {
        s << NAMEOF_ENUM(token.type) << "(" << token.lineIndex << "," << token.columnIndex << ")"
          << "\n";
    }
    if (tok != s.str()) {
        std::cout << "Actual:" << std::endl << s.str() << std::endl;
    }
    ASSERT_EQ(tok, s.str());
}

TEST(ScannerTest, IntegerLiteral) {
    auto tokens = tokenize("123");
    ASSERT_EQ(TokenKind::kNumberLiteral, tokens[0].type);
    ASSERT_EQ(1, tokens.size());
}

TEST(ScannerTest, MinusVsNegative) {
    auto tokens = tokenize("-4-5- -6 --7 -8.9");
    auto i = 0;

    ASSERT_EQ("-4", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(0, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kNumberLiteral, tokens[i++].type);

    ASSERT_EQ("-5", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(2, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kNumberLiteral, tokens[i++].type);

    ASSERT_EQ("-", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(4, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kMinusSign, tokens[i++].type);

    ASSERT_EQ("-6", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(6, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kNumberLiteral, tokens[i++].type);

    ASSERT_EQ("-", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(9, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kMinusSign, tokens[i++].type);

    ASSERT_EQ("-7", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(10, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kNumberLiteral, tokens[i++].type);

    ASSERT_EQ("-8.9", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(13, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kNumberLiteral, tokens[i++].type);

    ASSERT_EQ(i, tokens.size());
}

TEST(ScannerTest, ForLoop) {
    auto tokens = tokenize(
        "for i = 1 to 5\n"
        "    dim a = true\n"
        "next");
    auto i = 0;

    ASSERT_EQ("for", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(0, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kFor, tokens[i++].type);

    ASSERT_EQ("i", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(4, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kIdentifier, tokens[i++].type);

    ASSERT_EQ("=", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(6, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kEqualsSign, tokens[i++].type);

    ASSERT_EQ("1", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(8, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kNumberLiteral, tokens[i++].type);

    ASSERT_EQ("to", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(10, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kTo, tokens[i++].type);

    ASSERT_EQ("5", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(13, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kNumberLiteral, tokens[i++].type);

    ASSERT_EQ("\n", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(14, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kEndOfLine, tokens[i++].type);

    ASSERT_EQ("dim", tokens[i].text);
    ASSERT_EQ(1, tokens[i].lineIndex);
    ASSERT_EQ(4, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kDim, tokens[i++].type);

    ASSERT_EQ("a", tokens[i].text);
    ASSERT_EQ(1, tokens[i].lineIndex);
    ASSERT_EQ(8, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kIdentifier, tokens[i++].type);

    ASSERT_EQ("=", tokens[i].text);
    ASSERT_EQ(1, tokens[i].lineIndex);
    ASSERT_EQ(10, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kEqualsSign, tokens[i++].type);

    ASSERT_EQ("true", tokens[i].text);
    ASSERT_EQ(1, tokens[i].lineIndex);
    ASSERT_EQ(12, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kTrue, tokens[i++].type);

    ASSERT_EQ("\n", tokens[i].text);
    ASSERT_EQ(1, tokens[i].lineIndex);
    ASSERT_EQ(16, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kEndOfLine, tokens[i++].type);

    ASSERT_EQ("next", tokens[i].text);
    ASSERT_EQ(2, tokens[i].lineIndex);
    ASSERT_EQ(0, tokens[i].columnIndex);
    ASSERT_EQ(TokenKind::kNext, tokens[i++].type);

    ASSERT_EQ(i, tokens.size());
}

TEST(ScannerTest, ScanSimpleSub) {
    scanMatch("SimpleSub");
}
