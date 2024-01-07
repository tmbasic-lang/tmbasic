#pragma once

#include "../common.h"

namespace compiler {

enum class TokenKind {
    kError,
    kEndOfLine,
    kEndOfFile,
    kIdentifier,
    kComment,

    // literals
    kNumberLiteral,
    kStringLiteral,

    // punctuation
    kLeftParenthesis,
    kRightParenthesis,
    kLeftBracket,
    kRightBracket,
    kLeftBrace,
    kRightBrace,
    kColon,
    kSemicolon,
    kComma,
    kDot,
    kPlusSign,
    kMinusSign,
    kMultiplicationSign,
    kDivisionSign,
    kEqualsSign,
    kNotEqualsSign,
    kLessThanSign,
    kLessThanEqualsSign,
    kGreaterThanSign,
    kGreaterThanEqualsSign,
    kCaret,

    // keywords
    kAnd,
    kAs,
    kBoolean,
    kBy,
    kCall,
    kCase,
    kCatch,
    kConst,
    kContinue,
    kControl,
    kDate,
    kDateTime,
    kDateTimeOffset,
    kDim,
    kDo,
    kEach,
    kElse,
    kEnd,
    kExit,
    kFalse,
    kFinally,
    kFor,
    kForm,
    kFrom,
    kFunction,
    kGroup,
    kIf,
    kIn,
    kInput,
    kInto,
    kJoin,
    kKey,
    kList,
    kLoop,
    kMap,
    kMod,
    kNext,
    kNo,
    kNot,
    kNumber,
    kOf,
    kOn,
    kOptional,
    kOr,
    kPrint,
    kRecord,
    kRethrow,
    kReturn,
    kSelect,
    kSet,
    kShared,
    kStep,
    kSub,
    kString,
    kThen,
    kThrow,
    kTimeSpan,
    kTimeZone,
    kTo,
    kTrue,
    kTry,
    kType,
    kWend,
    kWhere,
    kWhile,
    kWith,
    kYield,
};

}  // namespace compiler
