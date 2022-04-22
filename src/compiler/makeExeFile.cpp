#include "makeExeFile.h"
#include "gzip.h"

using std::array;
using std::ofstream;
using std::runtime_error;
using std::string;
using std::vector;

// linux_arm32
extern uint8_t kResource_runner_linux_arm32_gz[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_arm32_gz_len;

// linux_arm64
extern uint8_t kResource_runner_linux_arm64_gz[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_arm64_gz_len;

// linux_x86
extern uint8_t kResource_runner_linux_x86_gz[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_x86_gz_len;

// linux_x64
extern uint8_t kResource_runner_linux_x64_gz[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_x64_gz_len;

// mac_x64
extern uint8_t kResource_runner_mac_x64_gz[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_mac_x64_gz_len;

// mac_arm64
extern uint8_t kResource_runner_mac_arm64_gz[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_mac_arm64_gz_len;

// win_x86
extern uint8_t kResource_runner_win_x86_gz[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_win_x86_gz_len;

// win_x64
extern uint8_t kResource_runner_win_x64_gz[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_win_x64_gz_len;

namespace compiler {

static vector<uint8_t> getRunnerTemplateGzipped(TargetPlatform platform) {
    vector<uint8_t> vec{};
    const uint8_t* data{};
    uint len{};
    switch (platform) {
        case TargetPlatform::kLinuxArm32:
            data = kResource_runner_linux_arm32_gz;
            len = kResource_runner_linux_arm32_gz_len;
            break;

        case TargetPlatform::kLinuxArm64:
            data = kResource_runner_linux_arm64_gz;
            len = kResource_runner_linux_arm64_gz_len;
            break;

        case TargetPlatform::kLinuxX86:
            data = kResource_runner_linux_x86_gz;
            len = kResource_runner_linux_x86_gz_len;
            break;

        case TargetPlatform::kLinuxX64:
            data = kResource_runner_linux_x64_gz;
            len = kResource_runner_linux_x64_gz_len;
            break;

        case TargetPlatform::kMacX64:
            data = kResource_runner_mac_x64_gz;
            len = kResource_runner_mac_x64_gz_len;
            break;

        case TargetPlatform::kMacArm64:
            data = kResource_runner_mac_arm64_gz;
            len = kResource_runner_mac_arm64_gz_len;
            break;

        case TargetPlatform::kWinX86:
            data = kResource_runner_win_x86_gz;
            len = kResource_runner_win_x86_gz_len;
            break;

        case TargetPlatform::kWinX64:
            data = kResource_runner_win_x64_gz;
            len = kResource_runner_win_x64_gz_len;
            break;

        default:
            assert(false);
            return vec;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    vec.insert(vec.end(), data, data + len);
    return vec;
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

vector<uint8_t> readRunnerFile() {
    // Read the contents of /code/bin/runner and return it
    vector<uint8_t> runnerBinary{};
    ifstream runnerFile{ "/code/bin/runner", ios::binary };
    if (!runnerFile.is_open()) {
        throw runtime_error("Cannot open /code/bin/runner");
    }
    runnerFile.seekg(0, ios::end);
    runnerBinary.resize(runnerFile.tellg());
    runnerFile.seekg(0, ios::beg);
    runnerFile.read(reinterpret_cast<char*>(runnerBinary.data()), runnerBinary.size());
    runnerFile.close();
    return runnerBinary;
}

vector<uint8_t> makeExeFile(const vector<uint8_t>& bytecode, TargetPlatform platform) {
    auto runnerBinaryGzip = getRunnerTemplateGzipped(platform);

    auto runnerBinary = runnerBinaryGzip.empty() ? readRunnerFile() : gunzip(runnerBinaryGzip);

    auto bytecodeIndex = findBytecodeIndex(runnerBinary);
    (void)bytecodeIndex;
    memcpy(&runnerBinary.at(bytecodeIndex), bytecode.data(), bytecode.size());
    return runnerBinary;
}

}  // namespace compiler
