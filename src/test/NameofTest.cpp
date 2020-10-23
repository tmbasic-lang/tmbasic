#include "common.h"
#include "gtest/gtest.h"

class BaseClass {
   public:
    virtual ~BaseClass() {}
};

class TestClass : public BaseClass {
   public:
    int memberVariable;
};

enum class TestEnum { kValue };

TEST(NameofTest, BaseClassName) {
    ASSERT_EQ("BaseClass", NAMEOF_TYPE(BaseClass));
}

TEST(NameofTest, TestClassName) {
    ASSERT_EQ("TestClass", NAMEOF_TYPE(TestClass));
}

TEST(NameofTest, MemberVariableName) {
    TestClass x;
    ASSERT_EQ("memberVariable", NAMEOF(x.memberVariable));
}

TEST(NameofTest, EnumLiteralName) {
    ASSERT_EQ("kValue", NAMEOF_ENUM(TestEnum::kValue));
}

TEST(NameofTest, EnumVariableName) {
    auto x = TestEnum::kValue;
    ASSERT_EQ("kValue", NAMEOF_ENUM(x));
}
