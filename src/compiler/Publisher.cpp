#include "Publisher.h"
#include "compiler/makeExeFile.h"
#include "compiler/gzip.h"
#include "compiler/zip.h"
#include "shared/path.h"
#include "shared/tar.h"

namespace compiler {

Publisher::Publisher(const CompiledProgram& compiledProgram, const std::string& basFilePath)
    : _filename(shared::getFileNameWithoutExtension(basFilePath)),
      _publishDir(getPublishDir(basFilePath)),
      _pcode(compiledProgram.serialize()) {}

std::string Publisher::getPublishDir(const std::string& basFilePath) {
    std::string const basDir = shared::getDirectoryName(basFilePath);
    return shared::pathCombine(basDir, "publish");
}

std::string Publisher::getPublishDir() const {
    return _publishDir;
}

std::string Publisher::publish(TargetPlatform platform) {
    shared::createDirectory(_publishDir);
    auto isZip = compiler::getTargetPlatformArchiveType(platform) == compiler::TargetPlatformArchiveType::kZip;
    auto archiveFilename =
        fmt::format("{}-{}.{}", _filename, compiler::getPlatformName(platform), isZip ? "zip" : "tar.gz");
    auto archiveFilePath = shared::pathCombine(_publishDir, archiveFilename);
    auto exeData = compiler::makeExeFile(_pcode, platform);
    auto exeFilename = fmt::format("{}{}", _filename, compiler::getPlatformExeExtension(platform));
    const std::string licFilename{ "LICENSE.txt" };
    auto licString = compiler::getLicenseForPlatform(platform);
    std::vector<uint8_t> licData{};
    licData.insert(licData.end(), licString.begin(), licString.end());
    if (isZip) {
        std::vector<compiler::ZipEntry> const entries{
            compiler::ZipEntry{ std::move(exeFilename), std::move(exeData) },
            compiler::ZipEntry{ licFilename, std::move(licData) },
        };
        compiler::zip(archiveFilePath, entries);
    } else {
        std::vector<shared::TarEntry> const entries{
            shared::TarEntry{ std::move(exeFilename), std::move(exeData), 0777 },
            shared::TarEntry{ licFilename, std::move(licData), 0664 },
        };
        auto gz = compiler::gzip(shared::tar(entries));
        std::ofstream f{ archiveFilePath, std::ios::out | std::ios::binary };
        f.write(reinterpret_cast<const char*>(gz.data()), static_cast<std::streamsize>(gz.size()));
    }
    return archiveFilePath;
}

}  // namespace compiler
