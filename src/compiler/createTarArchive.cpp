#include "createTarArchive.h"
#include <microtar.h>

namespace compiler {

TarEntry::TarEntry(std::string name, std::vector<uint8_t> data, uint mode)
    : name(std::move(name)), data(std::move(data)), mode(mode) {}

class TarArchive {
   public:
    std::vector<uint8_t> bytes{};
    size_t index{ 0 };

    static int read(mtar_t* tar, void* data, unsigned size) {
        auto* archive = reinterpret_cast<TarArchive*>(tar->stream);
        if (archive->index + size > archive->bytes.size()) {
            return MTAR_EREADFAIL;
        }
        memcpy(data, &archive->bytes.at(archive->index), size);
        archive->index += size;
        return MTAR_ESUCCESS;
    }

    static int write(mtar_t* tar, const void* data, unsigned size) {
        const auto* writeBytes = reinterpret_cast<const uint8_t*>(data);
        auto* archive = reinterpret_cast<TarArchive*>(tar->stream);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        archive->bytes.insert(archive->bytes.begin() + archive->index, writeBytes, &writeBytes[size]);
        archive->index += size;
        return MTAR_ESUCCESS;
    }

    static int seek(mtar_t* tar, unsigned pos) {
        auto* archive = reinterpret_cast<TarArchive*>(tar->stream);
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

std::vector<uint8_t> createTarArchive(const std::vector<TarEntry>& entries) {
    TarArchive archive{};
    mtar_t mtar{};
    mtar.read = TarArchive::read;
    mtar.write = TarArchive::write;
    mtar.seek = TarArchive::seek;
    mtar.close = TarArchive::close;
    mtar.stream = &archive;
    for (const auto& entry : entries) {
        if (TarArchive::writeFileHeader(&mtar, entry.name, entry.data.size(), entry.mode) < 0) {
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

}  // namespace compiler
