#include "makeExeFile.h"
#include "shared/process.h"
#include "shared/filesystem.h"
#include "shared/ExecutablePatcher.h"

using std::ifstream;
using std::ios;
using std::runtime_error;
using std::string;
using std::vector;

namespace compiler {

static vector<uint8_t> loadRuntimeFile(TargetPlatform platform) {
    string os;
    string arch;

    switch (platform) {
        case TargetPlatform::kLinuxArm32:
            os = "linux";
            arch = "arm32";
            break;
        case TargetPlatform::kLinuxArm64:
            os = "linux";
            arch = "arm64";
            break;
        case TargetPlatform::kLinuxX86:
            os = "linux";
            arch = "x86";
            break;
        case TargetPlatform::kLinuxX64:
            os = "linux";
            arch = "x64";
            break;
        case TargetPlatform::kMacX64:
            os = "mac";
            arch = "x64";
            break;
        case TargetPlatform::kMacArm64:
            os = "mac";
            arch = "arm64";
            break;
        case TargetPlatform::kWinX86:
            os = "win";
            arch = "x86";
            break;
        case TargetPlatform::kWinX64:
            os = "win";
            arch = "x64";
            break;
        case TargetPlatform::kWinArm64:
            os = "win";
            arch = "arm64";
            break;
        default:
            assert(false);
            return {};
    }

    string const runtimePath = shared::getExecutableDirectoryPath() + "/runtime_" + os + "_" + arch + ".dat";

    ifstream runtimeFile(runtimePath, ios::binary);
    if (!runtimeFile.is_open()) {
        throw runtime_error("Could not open runtime file: " + runtimePath);
    }

    runtimeFile.seekg(0, ios::end);
    vector<uint8_t> runtimeBinary(runtimeFile.tellg());
    runtimeFile.seekg(0, ios::beg);
    runtimeFile.read(reinterpret_cast<char*>(runtimeBinary.data()), static_cast<std::streamsize>(runtimeBinary.size()));
    runtimeFile.close();

    return runtimeBinary;
}

static std::vector<uint8_t> loadTzdbFile() {
    auto tarFilePath = shared::getExecutableDirectoryPath() + "/tzdb.dat";
    std::ifstream tarFile(tarFilePath, std::ios::binary);
    if (!tarFile.is_open()) {
        throw runtime_error("Could not open tzdb file: " + tarFilePath);
    }
    std::vector<uint8_t> tzdbData(std::istreambuf_iterator<char>(tarFile), {});
    tarFile.close();
    return tzdbData;
}

static std::vector<uint8_t> readBytesFromFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    std::vector<uint8_t> bytes(std::istreambuf_iterator<char>(file), {});
    file.close();
    return bytes;
}

static void writeBytesToFile(const std::string& filePath, const std::vector<uint8_t>& bytes) {
    std::ofstream file(filePath, std::ios::out | std::ios::binary);
    file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    file.close();
}

vector<uint8_t> makeExeFile(const vector<uint8_t>& bytecode, TargetPlatform platform) {
    auto runtimeBytes = loadRuntimeFile(platform);
    auto tzdbBytes = loadTzdbFile();

    // LIEF operates on-disk, so we'll need a temporary file.
    auto tempFilePath = shared::getTempFilePath("tmbasic-program.tmp");
    writeBytesToFile(tempFilePath, runtimeBytes);

    std::vector<uint8_t> exeBytes;
    try {
        // Our ExecutablePatcher will write the bytecode and tzdb to the EXE file.
        shared::ExecutablePatcher patcher{ tempFilePath };
        patcher.addResource(shared::ExecutablePatcher::kResourcePcode, bytecode);
        patcher.addResource(shared::ExecutablePatcher::kResourceTzdb, tzdbBytes);
        patcher.save();

        // Read the EXE file back into memory.
        exeBytes = readBytesFromFile(tempFilePath);
    } catch (...) {
        shared::deleteFile(tempFilePath);
        throw;
    }

    shared::deleteFile(tempFilePath);
    return exeBytes;
}

}  // namespace compiler
