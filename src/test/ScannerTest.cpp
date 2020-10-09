#include "gtest/gtest.h"
#include "core/compiler/Scanner.h"
#include "core/basic/TokenType.h"

using namespace basic;
using namespace compiler;

TEST(ScannerTest, IntegerLiteral) {
    auto tokens = Scanner::tokenize("123");
    ASSERT_EQ(TokenType::kIntegerLiteral, tokens[0].type);
    ASSERT_EQ(1, tokens.size());
}

TEST(ScannerTest, MinusVsNegative) {
    auto tokens = Scanner::tokenize("-4-5- -6 --7 -8.9");
    auto i = 0;

    ASSERT_EQ("-4", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(0, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kIntegerLiteral, tokens[i++].type);

    ASSERT_EQ("-5", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(2, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kIntegerLiteral, tokens[i++].type);

    ASSERT_EQ("-", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(4, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kMinusSign, tokens[i++].type);

    ASSERT_EQ("-6", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(6, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kIntegerLiteral, tokens[i++].type);

    ASSERT_EQ("-", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(9, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kMinusSign, tokens[i++].type);

    ASSERT_EQ("-7", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(10, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kIntegerLiteral, tokens[i++].type);

    ASSERT_EQ("-8.9", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(13, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kDecimalLiteral, tokens[i++].type);

    ASSERT_EQ(i, tokens.size());
}

TEST(ScannerTest, ForLoop) {
    auto tokens = Scanner::tokenize(
        "for i = 1 to 5\n"
        "    dim a = true\n"
        "next");
    auto i = 0;

    ASSERT_EQ("for", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(0, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kFor, tokens[i++].type);

    ASSERT_EQ("i", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(4, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kIdentifier, tokens[i++].type);

    ASSERT_EQ("=", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(6, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kEqualsSign, tokens[i++].type);

    ASSERT_EQ("1", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(8, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kIntegerLiteral, tokens[i++].type);

    ASSERT_EQ("to", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(10, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kTo, tokens[i++].type);

    ASSERT_EQ("5", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(13, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kIntegerLiteral, tokens[i++].type);

    ASSERT_EQ("\n", tokens[i].text);
    ASSERT_EQ(0, tokens[i].lineIndex);
    ASSERT_EQ(14, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kEndOfLine, tokens[i++].type);

    ASSERT_EQ("dim", tokens[i].text);
    ASSERT_EQ(1, tokens[i].lineIndex);
    ASSERT_EQ(4, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kDim, tokens[i++].type);

    ASSERT_EQ("a", tokens[i].text);
    ASSERT_EQ(1, tokens[i].lineIndex);
    ASSERT_EQ(8, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kIdentifier, tokens[i++].type);

    ASSERT_EQ("=", tokens[i].text);
    ASSERT_EQ(1, tokens[i].lineIndex);
    ASSERT_EQ(10, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kEqualsSign, tokens[i++].type);

    ASSERT_EQ("true", tokens[i].text);
    ASSERT_EQ(1, tokens[i].lineIndex);
    ASSERT_EQ(12, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kTrue, tokens[i++].type);

    ASSERT_EQ("\n", tokens[i].text);
    ASSERT_EQ(1, tokens[i].lineIndex);
    ASSERT_EQ(16, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kEndOfLine, tokens[i++].type);

    ASSERT_EQ("next", tokens[i].text);
    ASSERT_EQ(2, tokens[i].lineIndex);
    ASSERT_EQ(0, tokens[i].columnIndex);
    ASSERT_EQ(TokenType::kNext, tokens[i++].type);

    ASSERT_EQ(i, tokens.size());
}
