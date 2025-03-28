#include "makeExeFile.h"
#include "shared/process.h"

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

static size_t findBytecodeIndex(const vector<uint8_t>& runnerBinary) {
    // locate the "TTTTT..." block that we will replace with the bytecode
    constexpr size_t kSentinelLength = 16;
    auto sentinel = array<char, kSentinelLength>();
    memset(sentinel.data(), 'T', kSentinelLength);
    for (size_t i = 0; i < runnerBinary.size() - kSentinelLength; i += 32768) {
        if (runnerBinary[i] == 'T' && runnerBinary[i + 1] == 'T' &&
            memcmp(sentinel.data(), &runnerBinary[i], kSentinelLength) == 0) {
            while (runnerBinary[i] == 'T') {
                i--;
            }
            return i + 1;
        }
    }

    throw runtime_error("Cannot find the bytecode in the runner binary!");
}

vector<uint8_t> makeExeFile(const vector<uint8_t>& bytecode, TargetPlatform platform) {
    auto runnerBinary = loadRuntimeFile(platform);

    auto bytecodeIndex = findBytecodeIndex(runnerBinary);
    memcpy(&runnerBinary.at(bytecodeIndex), bytecode.data(), bytecode.size());
    return runnerBinary;
}

}  // namespace compiler
