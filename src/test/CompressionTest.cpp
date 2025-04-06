#include "../common.h"
#include "compiler/gzip.h"
#include "compiler/zip.h"
#include "gtest/gtest.h"
#include "helpers.h"
#include "shared/tar.h"

#include <microtar.h>

using compiler::gunzip;
using compiler::gzip;
using compiler::zip;
using compiler::ZipEntry;
using shared::tar;
using shared::TarEntry;

TEST(CompressionTest, Tar) {
    std::vector<TarEntry> const entries{
        TarEntry{ "a", std::vector<uint8_t>{ 1, 2, 3 }, 0777 },
        TarEntry{ "b", std::vector<uint8_t>{ 'H', 'i' }, 0664 },
    };
    auto tarBytes = tar(entries);
    ASSERT_EQ(3072UL, tarBytes.size());
}

TEST(CompressionTest, Gzip) {
    std::vector<uint8_t> const vec{ 1, 2, 3 };
    auto gz = gzip(vec);
    ASSERT_EQ(23UL, gz.size());
    auto orig = gunzip(gz);
    ASSERT_EQ(3UL, orig.size());
    ASSERT_EQ(1, orig.at(0));
    ASSERT_EQ(2, orig.at(1));
    ASSERT_EQ(3, orig.at(2));
}

TEST(CompressionTest, Zip) {
    std::vector<ZipEntry> const entries{
        ZipEntry{ "a", std::vector<uint8_t>{ 1, 2, 3 } },
        ZipEntry{ "b", std::vector<uint8_t>{ 'H', 'i' } },
    };
#ifdef _WIN32
    auto tempDir = std::filesystem::temp_directory_path();
    auto zipFilePath = (tempDir / "tmbasic_CompressionTest_CreateZipArchive.zip").string();
#else
    const std::string zipFilePath = "/tmp/tmbasic_CompressionTest_CreateZipArchive.zip";
#endif
    zip(zipFilePath, entries);
    struct stat st {};
    stat(zipFilePath.c_str(), &st);
    unlink(zipFilePath.c_str());
    ASSERT_EQ(187, st.st_size);
}

TEST(MicrotarTest, Roundtrip) {
    std::array<char, 5000> data{};
    for (size_t i = 0; i < data.size(); i++) {
        data.at(i) = static_cast<char>(i % 256);
    }

    // Write
    {
        mtar_t tar;

        mtar_open(&tar, "test.tar", "w");

        mtar_write_file_header(&tar, "test1.txt", static_cast<uint32_t>(data.size()));
        mtar_write_data(&tar, data.data(), static_cast<uint32_t>(data.size()));

        mtar_finalize(&tar);

        mtar_close(&tar);
    }

    // Read
    {
        mtar_t tar;
        mtar_header_t h;

        mtar_open(&tar, "test.tar", "r");

        while ((mtar_read_header(&tar, &h)) != MTAR_ENULLRECORD) {
            std::vector<char> buf(h.size);
            mtar_read_data(&tar, buf.data(), h.size);

            // Verify buf vs. data
            for (size_t i = 0; i < buf.size(); i++) {
                ASSERT_EQ(data.at(i), buf.at(i));
            }

            mtar_next(&tar);
        }

        mtar_close(&tar);
    }

    // Delete files
    unlink("test1.txt");
    unlink("test.tar");
}
