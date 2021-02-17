#include "patchBsdiff.h"
#include "decompressBz2.h"

using std::runtime_error;
using std::vector;

struct BspatchState {
    uint8_t* data;
    size_t index;
    size_t length;
};

// bspatch.h isn't valid C++ so let's just define the necessary stuff here ourselves in a C++-friendly format.
// while we're at it, we can substitute our own opaque pointer type in place of the void* used in the original.
struct bspatch_stream {
    BspatchState* opaque;
    int (*read)(const bspatch_stream* stream, void* buffer, int length);
};

extern "C" int bspatch(
    const uint8_t* oldData,
    int64_t oldLength,
    uint8_t* newData,
    int64_t newLength,
    bspatch_stream* stream);

namespace compiler {

static int64_t readNewLength(const uint8_t* buf) {
    int64_t length = 0;

    length = buf[7];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    length <<= 8;
    length |= buf[6];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    length <<= 8;
    length |= buf[5];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    length <<= 8;
    length |= buf[4];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    length <<= 8;
    length |= buf[3];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    length <<= 8;
    length |= buf[2];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    length <<= 8;
    length |= buf[1];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    length <<= 8;
    length |= buf[0];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    return length;
}

static int readDiff(const bspatch_stream* stream, void* buffer, int length) {
    if (stream->opaque->index + length > stream->opaque->length) {
        return -1;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    memcpy(buffer, &stream->opaque->data[stream->opaque->index], length);
    stream->opaque->index += length;
    return 0;
}

vector<uint8_t> patchBsdiff(const uint8_t* original, uint originalLength, const uint8_t* diff, uint diffLength) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto newLength = readNewLength(&diff[16]);
    auto newData = vector<uint8_t>(newLength);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto decompressedDiff = decompressBz2(&diff[24], diffLength - 24);
    auto state = BspatchState{ decompressedDiff.data(), 0, decompressedDiff.size() };
    auto stream = bspatch_stream{ &state, readDiff };
    auto result = bspatch(original, originalLength, newData.data(), newLength, &stream);
    if (result != 0) {
        throw runtime_error("Bspatch error!");
    }
    return newData;
}

}  // namespace compiler
