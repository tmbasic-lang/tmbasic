#include "../common.h"
#include "shared/Error.h"
#include "shared/path.h"
#include "gtest/gtest.h"
#include "helpers.h"

using shared::Error;
using shared::getDirectoryName;

TEST(PathTest, GetDirectoryName_EmptyPath) {
    std::string filePath = "";
    std::string expectedDir = ".";
    EXPECT_EQ(getDirectoryName(filePath), expectedDir);
}

TEST(PathTest, GetDirectoryName_NoDirectory) {
    std::string filePath = "hello";
    std::string expectedDir = ".";
    EXPECT_EQ(getDirectoryName(filePath), expectedDir);
}

#ifdef __APPLE__
// macOS-specific Tests
TEST(PathTest, GetDirectoryName_MacOSBehavior) {
    std::string filePath = "/path/to/file.txt";
    std::string expectedDir = "/path/to";
    EXPECT_EQ(getDirectoryName(filePath), expectedDir);
}
#endif

#ifdef _WIN32
// Windows-specific Tests
TEST(PathTest, GetDirectoryName_WindowsBehavior) {
    std::string filePath = "C:\\path\\to\\file.txt";
    std::string expectedDir = "C:\\path\\to";
    EXPECT_EQ(getDirectoryName(filePath), expectedDir);
}
#endif

#ifdef __linux__
// Linux-specific Tests
TEST(PathTest, GetDirectoryName_LinuxBehavior) {
    std::string filePath = "/path/to/file.txt";
    std::string expectedDir = "/path/to";
    EXPECT_EQ(getDirectoryName(filePath), expectedDir);
}
#endif
