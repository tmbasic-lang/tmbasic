#include "../common.h"
#include "gtest/gtest.h"

class BaseClass {
   public:
    virtual ~BaseClass() = default;

    // Rule of five implementation
    BaseClass() = default;
    BaseClass(const BaseClass&) = default;
    BaseClass& operator=(const BaseClass&) = default;
    BaseClass(BaseClass&&) = default;
    BaseClass& operator=(BaseClass&&) = default;
};

class TestClass : public BaseClass {
   public:
    int memberVariable{};
};

enum class TestEnum { kValue };

TEST(NameofTest, BaseClassName) {
#ifdef _WIN32
    ASSERT_EQ("class BaseClass", NAMEOF_TYPE(BaseClass));
#else
    ASSERT_EQ("BaseClass", NAMEOF_TYPE(BaseClass));
#endif
}

TEST(NameofTest, TestClassName) {
#ifdef _WIN32
    ASSERT_EQ("class TestClass", NAMEOF_TYPE(TestClass));
#else
    ASSERT_EQ("TestClass", NAMEOF_TYPE(TestClass));
#endif
}

TEST(NameofTest, MemberVariableName) {
    TestClass const x;
    ASSERT_EQ("memberVariable", NAMEOF(x.memberVariable));
}

TEST(NameofTest, EnumLiteralName) {
    ASSERT_EQ("kValue", NAMEOF_ENUM(TestEnum::kValue));
}

TEST(NameofTest, EnumVariableName) {
    auto x = TestEnum::kValue;
    ASSERT_EQ("kValue", NAMEOF_ENUM(x));
}
