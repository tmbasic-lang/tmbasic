#include "../common.h"
#include "compiler/tokenize.h"
#include "gtest/gtest.h"
#include "helpers.h"
#include "compiler/TokenKind.h"

using compiler::tokenize;
using compiler::TokenizeType;
using compiler::TokenKind;

TEST(ScannerTest, IntegerLiteral) {
    auto tokens = tokenize("123", TokenizeType::kFormat, nullptr);
    ASSERT_EQ(TokenKind::kNumberLiteral, tokens[0].type);
    ASSERT_EQ(1UL, tokens.size());
}

TEST(ScannerTest, MinusVsNegative) {
    auto tokens = tokenize("-4-5- -6 --7 -8.9", TokenizeType::kFormat, nullptr);
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

    ASSERT_EQ(static_cast<unsigned long>(i), tokens.size());
}

TEST(ScannerTest, ForLoop) {
    auto tokens = tokenize(
        "for i = 1 to 5\n"
        "    dim a = true\n"
        "next",
        TokenizeType::kFormat, nullptr);
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

    ASSERT_EQ(static_cast<unsigned long>(i), tokens.size());
}
