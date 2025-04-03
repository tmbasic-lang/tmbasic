#include "makeExeFile.h"
#include "shared/process.h"
#include "shared/runner_const.h"

using std::array;
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
        default:
            assert(false);
            return {};
    }

    string runtimePath = shared::getExecutableDirectoryPath() + "/runtime_" + os + "_" + arch + ".dat";

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

vector<uint8_t> makeExeFile(const vector<uint8_t>& bytecode, TargetPlatform platform) {
    auto runtimeBinary = loadRuntimeFile(platform);

    // Load the tzdb data
    auto tarFilePath = shared::getExecutableDirectoryPath() + "/tzdb.dat";
    std::ifstream tarFile(tarFilePath, std::ios::binary);
    if (!tarFile.is_open()) {
        throw runtime_error("Could not open tzdb file: " + tarFilePath);
    }
    std::vector<char> tzdbData(std::istreambuf_iterator<char>(tarFile), {});
    tarFile.close();

    // Create the final executable binary
    vector<uint8_t> exeFile;

    // 1. Start with the runtime binary
    exeFile.insert(exeFile.end(), runtimeBinary.begin(), runtimeBinary.end());

    // 2. Append the bytecode (pcode)
    exeFile.insert(exeFile.end(), bytecode.begin(), bytecode.end());

    // 3. Append the bytecode length (4 bytes)
    uint32_t bytecodeLength = static_cast<uint32_t>(bytecode.size());
    const uint8_t* byteLengthPtr = reinterpret_cast<const uint8_t*>(&bytecodeLength);
    exeFile.insert(exeFile.end(), byteLengthPtr, byteLengthPtr + sizeof(bytecodeLength));

    // 4. Append the tzdb data
    exeFile.insert(exeFile.end(), tzdbData.begin(), tzdbData.end());

    // 5. Append the tzdb data length (4 bytes)
    uint32_t tzdbLength = static_cast<uint32_t>(tzdbData.size());
    const uint8_t* tzdbLengthPtr = reinterpret_cast<const uint8_t*>(&tzdbLength);
    exeFile.insert(exeFile.end(), tzdbLengthPtr, tzdbLengthPtr + sizeof(tzdbLength));

    // 6. Append the sentinel value (4 bytes)
    uint32_t sentinel = shared::kPcodeSentinel;
    const uint8_t* sentinelPtr = reinterpret_cast<const uint8_t*>(&sentinel);
    exeFile.insert(exeFile.end(), sentinelPtr, sentinelPtr + sizeof(sentinel));

    return exeFile;
}

}  // namespace compiler
