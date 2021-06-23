#include "../common.h"
#include "compiler/gzip.h"
#include "compiler/tar.h"
#include "compiler/zip.h"
#include "gtest/gtest.h"
#include "helpers.h"

using compiler::gunzip;
using compiler::gzip;
using compiler::tar;
using compiler::TarEntry;
using compiler::zip;
using compiler::ZipEntry;

TEST(CompressionTest, Tar) {
    std::vector<TarEntry> entries{
        TarEntry{ "a", std::vector<uint8_t>{ 1, 2, 3 }, 0777 },
        TarEntry{ "b", std::vector<uint8_t>{ 'H', 'i' }, 0664 },
    };
    auto tarBytes = tar(entries);
    ASSERT_EQ(3072, tarBytes.size());
}

TEST(CompressionTest, Gzip) {
    std::vector<uint8_t> vec{ 1, 2, 3 };
    auto gz = gzip(vec);
    ASSERT_EQ(23, gz.size());
    auto orig = gunzip(gz);
    ASSERT_EQ(3, orig.size());
    ASSERT_EQ(1, orig.at(0));
    ASSERT_EQ(2, orig.at(1));
    ASSERT_EQ(3, orig.at(2));
}

TEST(CompressionTest, Zip) {
    std::vector<ZipEntry> entries{
        ZipEntry{ "a", std::vector<uint8_t>{ 1, 2, 3 } },
        ZipEntry{ "b", std::vector<uint8_t>{ 'H', 'i' } },
    };
    const std::string kZipFilePath = "/tmp/tmbasic_CompressionTest_CreateZipArchive.zip";
    zip(kZipFilePath, entries);
    struct stat st;
    stat(kZipFilePath.c_str(), &st);
    unlink(kZipFilePath.c_str());
    ASSERT_EQ(187, st.st_size);
}
