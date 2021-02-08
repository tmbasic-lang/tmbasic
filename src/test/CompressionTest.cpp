#include "../common.h"
#include "compiler/decompressBz2.h"
#include "compiler/patchBsdiff.h"
#include "gtest/gtest.h"
#include "helpers.h"

using compiler::decompressBz2;
using compiler::patchBsdiff;

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
