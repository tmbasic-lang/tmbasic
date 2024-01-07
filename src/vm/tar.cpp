#include "tar.h"
#include <microtar.h>

namespace vm {

TarEntry::TarEntry(std::string name, std::vector<uint8_t> data, uint mode)
    : name(std::move(name)), data(std::move(data)), mode(mode) {}

class OutputTarArchive {
   public:
    std::vector<uint8_t> bytes{};
    size_t index{ 0 };

    static int read(mtar_t* tar, void* data, unsigned size) {
        auto* archive = reinterpret_cast<OutputTarArchive*>(tar->stream);
        if (archive->index + size > archive->bytes.size()) {
            return MTAR_EREADFAIL;
        }
        memcpy(data, &archive->bytes.at(archive->index), size);
        archive->index += size;
        return MTAR_ESUCCESS;
    }

    static int write(mtar_t* tar, const void* data, unsigned size) {
        const auto* writeBytes = reinterpret_cast<const uint8_t*>(data);
        auto* archive = reinterpret_cast<OutputTarArchive*>(tar->stream);
        archive->bytes.insert(  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            archive->bytes.begin() + static_cast<ptrdiff_t>(archive->index), writeBytes, &writeBytes[size]);
        archive->index += size;
        return MTAR_ESUCCESS;
    }

    static int seek(mtar_t* tar, unsigned pos) {
        auto* archive = reinterpret_cast<OutputTarArchive*>(tar->stream);
        if (pos > archive->bytes.size()) {
            return MTAR_ESEEKFAIL;
        }
        archive->index = pos;
        return MTAR_ESUCCESS;
    }

    static int close(mtar_t* /*tar*/) { return MTAR_ESUCCESS; }

    static int writeFileHeader(mtar_t* tar, const std::string& name, uint size, uint mode) {
        mtar_header_t h{};
        if (name.size() >= sizeof(h.name)) {
            throw std::runtime_error(fmt::format("The name \"{}\" is too long.", name));
        }
        memcpy(h.name, name.data(), name.size() + 1);
        h.size = size;
        h.type = MTAR_TREG;
        h.mode = mode;
        h.mtime = time(nullptr);
        return mtar_write_header(tar, &h);
    }
};

std::vector<uint8_t> tar(const std::vector<TarEntry>& entries) {
    OutputTarArchive archive{};
    mtar_t mtar{};
    mtar.read = OutputTarArchive::read;
    mtar.write = OutputTarArchive::write;
    mtar.seek = OutputTarArchive::seek;
    mtar.close = OutputTarArchive::close;
    mtar.stream = &archive;
    for (const auto& entry : entries) {
        if (OutputTarArchive::writeFileHeader(&mtar, entry.name, entry.data.size(), entry.mode) < 0) {
            throw std::runtime_error(fmt::format("Failed to write the tar header for \"{}\".", entry.name));
        }
        if (mtar_write_data(&mtar, entry.data.data(), entry.data.size()) < 0) {
            throw std::runtime_error(fmt::format("Failed to write the tar data for \"{}\".", entry.name));
        }
    }
    if (mtar_finalize(&mtar) < 0) {
        throw std::runtime_error("Failed to finalize the tar archive.");
    }
    if (mtar_close(&mtar) < 0) {
        throw std::runtime_error("Failed to close the tar archive.");
    }
    return archive.bytes;
}

class InputTarArchive {
   public:
    const char* data;
    size_t size;
    size_t index{ 0 };

    InputTarArchive(const char* tarData, size_t tarSize) : data(tarData), size(tarSize) {}

    static int read(mtar_t* tar, void* data, unsigned size) {
        auto* archive = reinterpret_cast<InputTarArchive*>(tar->stream);
        if (archive->index + size > archive->size) {
            return MTAR_EREADFAIL;
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        memcpy(data, &archive->data[archive->index], size);
        archive->index += size;
        return MTAR_ESUCCESS;
    }

    static int write(mtar_t* /*tar*/, const void* /*data*/, unsigned /*size*/) { return MTAR_EWRITEFAIL; }

    static int seek(mtar_t* tar, unsigned pos) {
        auto* archive = reinterpret_cast<InputTarArchive*>(tar->stream);
        if (pos > archive->size) {
            return MTAR_ESEEKFAIL;
        }
        archive->index = pos;
        return MTAR_ESUCCESS;
    }

    static int close(mtar_t* /*tar*/) { return MTAR_ESUCCESS; }

    static int readFileHeader(mtar_t* tar, std::string* name, uint* size, uint* mode) {
        mtar_header_t h{};
        auto result = mtar_read_header(tar, &h);
        if (result < 0) {
            return result;
        }
        *name = h.name;
        *size = h.size;
        *mode = h.mode;
        return result;
    }
};

// Only use this for our own archives, created with microtar. It doesn't support all tar features.
void untar(
    const char* tarData,
    size_t tarSize,
    const std::function<void(const char* name, const char* data, size_t length)>& entryCallback) {
    InputTarArchive archive{ tarData, tarSize };
    mtar_t mtar{};
    memset(&mtar, 0, sizeof(mtar_t));
    mtar.read = InputTarArchive::read;
    mtar.write = InputTarArchive::write;
    mtar.seek = InputTarArchive::seek;
    mtar.close = InputTarArchive::close;
    mtar.stream = &archive;
    std::string name{};
    uint size{};
    uint mode{};
    while (InputTarArchive::readFileHeader(&mtar, &name, &size, &mode) == MTAR_ESUCCESS) {
        // Directories end in /, skip them.
        if (name.back() == '/') {
            mtar_next(&mtar);
            continue;
        }

        // Trim starting "./"
        if (name.size() >= 2 && name[0] == '.' && name[1] == '/') {
            name = name.substr(2);
        }

        std::vector<char> data{};
        data.resize(size);
        if (mtar_read_data(&mtar, data.data(), size) < 0) {
            throw std::runtime_error(fmt::format("Failed to read the tar data for \"{}\".", name));
        }
        entryCallback(name.c_str(), data.data(), data.size());

        mtar_next(&mtar);
    }
}

}  // namespace vm
