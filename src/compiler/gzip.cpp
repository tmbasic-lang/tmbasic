#include "gzip.h"
#define ZLIB_CONST
#include <zlib.h>

namespace compiler {

class ZStream {
   public:
    bool compress;
    z_stream stream{};

    explicit ZStream(bool compress) : compress(compress) {
        // + 16 means gzip format
        if (compress) {
            if (deflateInit2(&stream, Z_BEST_SPEED, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
                throw std::runtime_error("Failed to initialize zlib.");
            }
        } else {
            if (inflateInit2(&stream, 15 + 16) != Z_OK) {
                throw std::runtime_error("Failed to initialize zlib.");
            }
        }
    }

    ZStream(const ZStream&) = delete;
    ZStream& operator=(const ZStream&) = delete;
    ZStream(ZStream&&) = delete;
    ZStream& operator=(ZStream&&) = delete;

    ~ZStream() {
        if (compress) {
            deflateEnd(&stream);
        } else {
            inflateEnd(&stream);
        }
    }

    std::vector<uint8_t> run(const std::vector<uint8_t>& src) {
        constexpr size_t kChunk = 262144;
        std::array<uint8_t, kChunk> out{};
        std::vector<uint8_t> dst{};

        stream.next_in = src.data();
        stream.avail_in = static_cast<uInt>(src.size());

        int ret = 0;
        do {
            stream.avail_out = kChunk;
            stream.next_out = out.data();
            ret = compress ? deflate(&stream, Z_FINISH) : inflate(&stream, Z_FINISH);
            if (ret == Z_STREAM_ERROR) {
                throw std::runtime_error(compress ? "Gzip compression failed." : "Gzip decompression failed.");
            }
            auto have = kChunk - stream.avail_out;
            dst.insert(dst.end(), out.begin(), out.begin() + have);
        } while (ret != Z_STREAM_END || stream.avail_out != kChunk);
        assert(stream.avail_in == 0);

        return dst;
    }
};

std::vector<uint8_t> gzip(const std::vector<uint8_t>& src) {
    return ZStream{ true }.run(src);
}

std::vector<uint8_t> gunzip(const std::vector<uint8_t>& src) {
    return ZStream{ false }.run(src);
}

}  // namespace compiler
