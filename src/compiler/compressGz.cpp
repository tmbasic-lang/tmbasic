#include "compressGz.h"
#define ZLIB_CONST
#include <zlib.h>

namespace compiler {

class ZStream {
   public:
    z_stream stream{};
    ZStream() {
        // + 16 means gzip format
        if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            throw std::runtime_error("Failed to initialize zlib.");
        }
    }
    ZStream(const ZStream&) = delete;
    ZStream& operator=(const ZStream&) = delete;
    ZStream(ZStream&&) = delete;
    ZStream& operator=(ZStream&&) = delete;
    ~ZStream() { deflateEnd(&stream); }
};

std::vector<uint8_t> compressGz(const std::vector<uint8_t>& src) {
    constexpr size_t kChunk = 262144;
    ZStream s{};
    std::array<uint8_t, kChunk> out{};
    std::vector<uint8_t> dst{};

    s.stream.next_in = src.data();
    s.stream.avail_in = src.size();

    int ret = 0;
    do {
        s.stream.avail_out = kChunk;
        s.stream.next_out = out.data();
        ret = deflate(&s.stream, Z_FINISH);
        if (ret == Z_STREAM_ERROR) {
            throw std::runtime_error("Gzip compression failed.");
        }
        auto have = kChunk - s.stream.avail_out;
        dst.insert(dst.end(), out.begin(), out.begin() + have);
    } while (ret != Z_STREAM_END || s.stream.avail_out != kChunk);
    assert(s.stream.avail_in == 0);

    return dst;
}

}  // namespace compiler
