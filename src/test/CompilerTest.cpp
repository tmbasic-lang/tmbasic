#include "../common.h"
#include "gtest/gtest.h"
#include "compiler/TargetPlatform.h"
#include "compiler/makeExeFile.h"

using compiler::makeExeFile;
using compiler::TargetPlatform;
using std::runtime_error;
using std::string;
using std::vector;

static TargetPlatform getCurrentPlatform() {
    auto platform = string(getenv("TARGET_OS")) + "/" + getenv("ARCH");
    if (platform == "linux/i686") {
        return TargetPlatform::kLinuxX86;
    } else if (platform == "linux/x86_64") {
        return TargetPlatform::kLinuxX64;
    } else if (platform == "linux/arm32v7") {
        return TargetPlatform::kLinuxArm32;
    } else if (platform == "linux/arm64v8") {
        return TargetPlatform::kLinuxArm64;
    } else if (platform == "mac/x86_64") {
        return TargetPlatform::kMacX64;
    } else if (platform == "win/i686") {
        return TargetPlatform::kWinX86;
    } else if (platform == "win/x86_64") {
        return TargetPlatform::kWinX64;
    } else {
        throw runtime_error(string("Invalid platform: ") + platform);
    }
}

static vector<uint8_t> buildDummyBytecode(size_t length) {
    auto bytecode = vector<uint8_t>(length);
    for (size_t i = 0; i < length; i++) {
        bytecode.at(i) = i & 0xFF;
    }
    return bytecode;
}

static bool contains(const vector<uint8_t>& haystack, const vector<uint8_t>& needle) {
    for (size_t i = 0; i < haystack.size() - needle.size(); i++) {
        if (haystack.at(i) == needle.at(0) && memcmp(&haystack.at(i), needle.data(), needle.size()) == 0) {
            return true;
        }
    }

    return false;
}

TEST(CompilerTest, BuildDummyBytecode) {
    auto bytecode = buildDummyBytecode(1000);
    ASSERT_EQ(0, bytecode.at(0));
    ASSERT_EQ(1, bytecode.at(1));
    ASSERT_EQ(2, bytecode.at(2));
    ASSERT_EQ(0, bytecode.at(256));
    ASSERT_EQ(1, bytecode.at(257));
}

TEST(CompilerTest, Contains) {
    auto haystack = vector<uint8_t>{ 1, 2, 3, 4, 5, 6 };
    auto needle1 = vector<uint8_t>{ 3, 4 };
    auto needle2 = vector<uint8_t>{ 4, 4 };

    ASSERT_TRUE(contains(haystack, needle1));
    ASSERT_FALSE(contains(haystack, needle2));
}

TEST(CompilerTest, MakeExeFile102400) {
    auto bytecode = buildDummyBytecode(1000);
    auto exe = makeExeFile(bytecode, getCurrentPlatform());
    ASSERT_TRUE(contains(exe, bytecode));
}

TEST(CompilerTest, MakeExeFile524288) {
    auto bytecode = buildDummyBytecode(250000);
    auto exe = makeExeFile(bytecode, getCurrentPlatform());
    ASSERT_TRUE(contains(exe, bytecode));
}

TEST(CompilerTest, MakeExeFile1048576) {
    auto bytecode = buildDummyBytecode(750000);
    auto exe = makeExeFile(bytecode, getCurrentPlatform());
    ASSERT_TRUE(contains(exe, bytecode));
}

TEST(CompilerTest, MakeExeFile5242880) {
    auto bytecode = buildDummyBytecode(1100000);
    auto exe = makeExeFile(bytecode, getCurrentPlatform());
    ASSERT_TRUE(contains(exe, bytecode));
}
