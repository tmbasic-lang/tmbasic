#include "zip.h"
#include <zip.h>

namespace compiler {

ZipEntry::ZipEntry(std::string name, std::vector<uint8_t> data) : name(std::move(name)), data(std::move(data)) {}

struct ZipClose {
    void operator()(zip_t* zip) {
        if (zip) {
            zip_close(zip);
        }
    }
};

void zip(const std::string& zipFilePath, const std::vector<ZipEntry>& entries) {
    int zipError = 0;
    auto zip = std::unique_ptr<zip_t, ZipClose>(zip_open(zipFilePath.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &zipError));
    if (!zip) {
        throw std::runtime_error(fmt::format("Failed to create zip file. zip_open error #{}", zipError));
    }
    for (const auto& entry : entries) {
        auto* zipSource = zip_source_buffer(zip.get(), entry.data.data(), entry.data.size(), 0);
        if (!zipSource) {
            throw std::runtime_error(
                fmt::format("Failed to create zip file. zip_source_buffer_create error: {}", zip_strerror(zip.get())));
        }
        auto fileIndex = zip_file_add(zip.get(), entry.name.c_str(), zipSource, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8);
        if (fileIndex == -1) {
            throw std::runtime_error(
                fmt::format("Failed to create zip file. zip_file_add error: {}", zip_strerror(zip.get())));
        }
        zip_set_file_compression(zip.get(), fileIndex, ZIP_CM_DEFLATE, 1 /* fastest */);
    }
}

}  // namespace compiler
