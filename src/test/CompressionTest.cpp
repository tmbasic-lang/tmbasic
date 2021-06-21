#include "../common.h"
#include "compiler/compressGz.h"
#include "compiler/createTarArchive.h"
#include "compiler/decompressBz2.h"
#include "compiler/patchBsdiff.h"
#include "gtest/gtest.h"
#include "helpers.h"

using compiler::compressGz;
using compiler::createTarArchive;
using compiler::decompressBz2;
using compiler::patchBsdiff;
using compiler::TarEntry;

TEST(CompressionTest, DecompressBz2) {
    auto compressed = readBinaryFile("bziptest.bz2");
    auto decompressed = decompressBz2(compressed.data(), compressed.size());
    ASSERT_EQ(100000, decompressed.size());
    for (size_t i = 0; i < decompressed.size(); i++) {
        ASSERT_EQ('T', decompressed.at(i));
    }
}

TEST(CompressionTest, PatchBsdiff) {
    auto oldData = readBinaryFile("bsdifftest.old");
    auto diffData = readBinaryFile("bsdifftest.bsdiff");
    auto expectedNewData = readBinaryFile("bsdifftest.new");
    auto actualNewData = patchBsdiff(oldData.data(), oldData.size(), diffData.data(), diffData.size());
    ASSERT_EQ(expectedNewData.size(), actualNewData.size());
    for (size_t i = 0; i < expectedNewData.size(); i++) {
        ASSERT_EQ(expectedNewData.at(i), actualNewData.at(i));
    }
}

TEST(CompressionTest, CreateTarArchive) {
    std::vector<TarEntry> entries{
        TarEntry{ "a", std::vector<uint8_t>{ 1, 2, 3 }, 0777 },
        TarEntry{ "b", std::vector<uint8_t>{ 'H', 'i' }, 0664 },
    };
    auto tarBytes = createTarArchive(entries);
    ASSERT_EQ(3072, tarBytes.size());
}

TEST(CompressionTest, CompressGz) {
    std::vector<uint8_t> vec{ 1, 2, 3 };
    auto gz = compressGz(vec);
    ASSERT_EQ(23, gz.size());
}
