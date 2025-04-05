#include "../common.h"
#include "shared/Error.h"
#include "shared/path.h"
#include "gtest/gtest.h"
#include "helpers.h"

using shared::getDirectoryName;
using shared::getExtension;
using shared::getFileName;
using shared::getFileNameWithoutExtension;
using shared::pathCombine;

TEST(PathTest, GetDirectoryName_EmptyPath) {
    std::string const filePath;
    std::string const expectedDir = ".";
    EXPECT_EQ(getDirectoryName(filePath), expectedDir);
}

TEST(PathTest, GetDirectoryName_NoDirectory) {
    std::string const filePath = "hello";
    std::string const expectedDir = ".";
    EXPECT_EQ(getDirectoryName(filePath), expectedDir);
}

#ifdef __APPLE__
// macOS-specific Tests
TEST(PathTest, GetDirectoryName_MacOSBehavior) {
    std::string filePath = "/path/to/file.txt";
    std::string expectedDir = "/path/to";
    EXPECT_EQ(getDirectoryName(filePath), expectedDir);
}

TEST(PathTest, GetFileName_MacOSBehavior) {
    std::string filePath = "/path/to/file.txt";
    std::string expectedFile = "file.txt";
    EXPECT_EQ(getFileName(filePath), expectedFile);
}

TEST(PathTest, GetFileNameWithoutExtension_MacOSBehavior) {
    std::string filePath = "/path/to/file.txt";
    std::string expectedFile = "file";
    EXPECT_EQ(getFileNameWithoutExtension(filePath), expectedFile);
}

TEST(PathTest, GetExtension_MacOSBehavior) {
    std::string filePath = "/path/to/file.txt";
    std::string expectedFile = ".txt";
    EXPECT_EQ(getExtension(filePath), expectedFile);
}

TEST(PathTest, PathCombine_MacOSBehavior) {
    std::string a = "/path/to";
    std::string b = "file.txt";
    std::string expected = "/path/to/file.txt";
    EXPECT_EQ(pathCombine(a, b), expected);
}
#endif

#ifdef _WIN32
// Windows-specific Tests
TEST(PathTest, GetDirectoryName_WindowsBehavior) {
    std::string filePath = "C:\\path\\to\\file.txt";
    std::string expectedDir = "C:\\path\\to";
    EXPECT_EQ(getDirectoryName(filePath), expectedDir);
}

TEST(PathTest, GetFileName_WindowsBehavior) {
    std::string filePath = "C:\\path\\to\\file.txt";
    std::string expectedFile = "file.txt";
    EXPECT_EQ(getFileName(filePath), expectedFile);
}

TEST(PathTest, GetFileNameWithoutExtension_WindowsBehavior) {
    std::string filePath = "C:\\path\\to\\file.txt";
    std::string expectedFile = "file";
    EXPECT_EQ(getFileNameWithoutExtension(filePath), expectedFile);
}

TEST(PathTest, GetExtension_WindowsBehavior) {
    std::string filePath = "C:\\path\\to\\file.txt";
    std::string expectedFile = ".txt";
    EXPECT_EQ(getExtension(filePath), expectedFile);
}

TEST(PathTest, PathCombine_WindowsBehavior) {
    std::string a = "C:\\path\\to";
    std::string b = "file.txt";
    std::string expected = "C:\\path\\to\\file.txt";
    EXPECT_EQ(pathCombine(a, b), expected);
}
#endif

#ifdef __linux__
// Linux-specific Tests
TEST(PathTest, GetDirectoryName_LinuxBehavior) {
    std::string const filePath = "/path/to/file.txt";
    std::string const expectedDir = "/path/to";
    EXPECT_EQ(getDirectoryName(filePath), expectedDir);
}

TEST(PathTest, GetFileName_LinuxBehavior) {
    std::string const filePath = "/path/to/file.txt";
    std::string const expectedFile = "file.txt";
    EXPECT_EQ(getFileName(filePath), expectedFile);
}

TEST(PathTest, GetFileNameWithoutExtension_LinuxBehavior) {
    std::string const filePath = "/path/to/file.txt";
    std::string const expectedFile = "file";
    EXPECT_EQ(getFileNameWithoutExtension(filePath), expectedFile);
}

TEST(PathTest, GetExtension_LinuxBehavior) {
    std::string const filePath = "/path/to/file.txt";
    std::string const expectedFile = ".txt";
    EXPECT_EQ(getExtension(filePath), expectedFile);
}

TEST(PathTest, PathCombine_LinuxBehavior) {
    std::string const a = "/path/to";
    std::string const b = "file.txt";
    std::string const expected = "/path/to/file.txt";
    EXPECT_EQ(pathCombine(a, b), expected);
}
#endif
