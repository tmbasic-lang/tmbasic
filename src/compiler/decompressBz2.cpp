#include "decompressBz2.h"
#include <bzlib.h>

using std::array;
using std::runtime_error;
using std::vector;

namespace compiler {

class BzStream {
   public:
    bz_stream stream = { nullptr };

    BzStream() {
        auto bzResult = BZ2_bzDecompressInit(&stream, 0, 0);
        if (bzResult != BZ_OK) {
            throw runtime_error("Bzip2 error!");
        }
    }

    BzStream(const BzStream&) = delete;
    BzStream& operator=(const BzStream&) = delete;
    BzStream(BzStream&&) = delete;
    BzStream& operator=(BzStream&&) = delete;

    ~BzStream() { BZ2_bzDecompressEnd(&stream); }
};

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
vector<uint8_t> decompressBz2(const uint8_t compressedData[], uint compressedLength) {
    vector<uint8_t> decompressedData;
    int bzResult = BZ_OK;
    BzStream bz;

    constexpr size_t bufferLength = 65536;
    auto buffer = array<char, bufferLength>();

    // bzip2 is very naughty and does not use "const" for input
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast,cppcoreguidelines-pro-type-reinterpret-cast)
    bz.stream.next_in = reinterpret_cast<char*>(const_cast<uint8_t*>(compressedData));
    bz.stream.avail_in = compressedLength;

    do {
        bz.stream.next_out = buffer.data();
        bz.stream.avail_out = bufferLength;

        bzResult = BZ2_bzDecompress(&bz.stream);
        if (bzResult != BZ_OK && bzResult != BZ_STREAM_END) {
            throw runtime_error("Bzip2 error!");
        }

        auto bufferFilledLength = bufferLength - bz.stream.avail_out;
        decompressedData.insert(
            decompressedData.end(), buffer.data(),
            bufferFilledLength == bufferLength ? buffer.end() : &buffer.at(bufferFilledLength));
    } while (bzResult != BZ_STREAM_END);

    return decompressedData;
}

}  // namespace compiler
