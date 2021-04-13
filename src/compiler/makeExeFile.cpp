#include "makeExeFile.h"
#include "decompressBz2.h"
#include "patchBsdiff.h"

using std::array;
using std::ofstream;
using std::runtime_error;
using std::string;
using std::vector;

// linux_arm32
extern uint8_t kResource_runner_linux_arm32_102400_bz2[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_arm32_102400_bz2_len;
extern uint8_t kResource_runner_linux_arm32_524288_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_arm32_524288_bsdiff_len;
extern uint8_t kResource_runner_linux_arm32_1048576_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_arm32_1048576_bsdiff_len;
extern uint8_t kResource_runner_linux_arm32_5242880_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_arm32_5242880_bsdiff_len;

// linux_arm64
extern uint8_t kResource_runner_linux_arm64_102400_bz2[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_arm64_102400_bz2_len;
extern uint8_t kResource_runner_linux_arm64_524288_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_arm64_524288_bsdiff_len;
extern uint8_t kResource_runner_linux_arm64_1048576_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_arm64_1048576_bsdiff_len;
extern uint8_t kResource_runner_linux_arm64_5242880_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_arm64_5242880_bsdiff_len;

// linux_x86
extern uint8_t kResource_runner_linux_x86_102400_bz2[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_x86_102400_bz2_len;
extern uint8_t kResource_runner_linux_x86_524288_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_x86_524288_bsdiff_len;
extern uint8_t kResource_runner_linux_x86_1048576_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_x86_1048576_bsdiff_len;
extern uint8_t kResource_runner_linux_x86_5242880_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_x86_5242880_bsdiff_len;

// linux_x64
extern uint8_t kResource_runner_linux_x64_102400_bz2[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_x64_102400_bz2_len;
extern uint8_t kResource_runner_linux_x64_524288_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_x64_524288_bsdiff_len;
extern uint8_t kResource_runner_linux_x64_1048576_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_x64_1048576_bsdiff_len;
extern uint8_t kResource_runner_linux_x64_5242880_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_linux_x64_5242880_bsdiff_len;

// mac_x64
extern uint8_t kResource_runner_mac_x64_102400_bz2[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_mac_x64_102400_bz2_len;
extern uint8_t kResource_runner_mac_x64_524288_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_mac_x64_524288_bsdiff_len;
extern uint8_t kResource_runner_mac_x64_1048576_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_mac_x64_1048576_bsdiff_len;
extern uint8_t kResource_runner_mac_x64_5242880_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_mac_x64_5242880_bsdiff_len;

// mac_arm64
extern uint8_t kResource_runner_mac_arm64_102400_bz2[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_mac_arm64_102400_bz2_len;
extern uint8_t kResource_runner_mac_arm64_524288_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_mac_arm64_524288_bsdiff_len;
extern uint8_t kResource_runner_mac_arm64_1048576_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_mac_arm64_1048576_bsdiff_len;
extern uint8_t kResource_runner_mac_arm64_5242880_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_mac_arm64_5242880_bsdiff_len;

// win_x86
extern uint8_t kResource_runner_win_x86_102400_bz2[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_win_x86_102400_bz2_len;
extern uint8_t kResource_runner_win_x86_524288_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_win_x86_524288_bsdiff_len;
extern uint8_t kResource_runner_win_x86_1048576_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_win_x86_1048576_bsdiff_len;
extern uint8_t kResource_runner_win_x86_5242880_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_win_x86_5242880_bsdiff_len;

// win_x64
extern uint8_t kResource_runner_win_x64_102400_bz2[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_win_x64_102400_bz2_len;
extern uint8_t kResource_runner_win_x64_524288_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_win_x64_524288_bsdiff_len;
extern uint8_t kResource_runner_win_x64_1048576_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_win_x64_1048576_bsdiff_len;
extern uint8_t kResource_runner_win_x64_5242880_bsdiff[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kResource_runner_win_x64_5242880_bsdiff_len;

namespace compiler {

struct ResourceData {
    const uint8_t* data;
    uint len;
};

static ResourceData getRunnerCompressedData(TargetPlatform platform, int size) {
    switch (platform) {
        case TargetPlatform::kLinuxArm32:
            if (size == 102400) {
                return { kResource_runner_linux_arm32_102400_bz2, kResource_runner_linux_arm32_102400_bz2_len };
            } else if (size == 524288) {
                return { kResource_runner_linux_arm32_524288_bsdiff, kResource_runner_linux_arm32_524288_bsdiff_len };
            } else if (size == 1048576) {
                return { kResource_runner_linux_arm32_1048576_bsdiff, kResource_runner_linux_arm32_1048576_bsdiff_len };
            } else if (size == 5242880) {
                return { kResource_runner_linux_arm32_5242880_bsdiff, kResource_runner_linux_arm32_5242880_bsdiff_len };
            } else {
                assert(false);
                return { nullptr, 0 };
            }

        case TargetPlatform::kLinuxArm64:
            if (size == 102400) {
                return { kResource_runner_linux_arm64_102400_bz2, kResource_runner_linux_arm64_102400_bz2_len };
            } else if (size == 524288) {
                return { kResource_runner_linux_arm64_524288_bsdiff, kResource_runner_linux_arm64_524288_bsdiff_len };
            } else if (size == 1048576) {
                return { kResource_runner_linux_arm64_1048576_bsdiff, kResource_runner_linux_arm64_1048576_bsdiff_len };
            } else if (size == 5242880) {
                return { kResource_runner_linux_arm64_5242880_bsdiff, kResource_runner_linux_arm64_5242880_bsdiff_len };
            } else {
                assert(false);
                return { nullptr, 0 };
            }

        case TargetPlatform::kLinuxX86:
            if (size == 102400) {
                return { kResource_runner_linux_x86_102400_bz2, kResource_runner_linux_x86_102400_bz2_len };
            } else if (size == 524288) {
                return { kResource_runner_linux_x86_524288_bsdiff, kResource_runner_linux_x86_524288_bsdiff_len };
            } else if (size == 1048576) {
                return { kResource_runner_linux_x86_1048576_bsdiff, kResource_runner_linux_x86_1048576_bsdiff_len };
            } else if (size == 5242880) {
                return { kResource_runner_linux_x86_5242880_bsdiff, kResource_runner_linux_x86_5242880_bsdiff_len };
            } else {
                assert(false);
                return { nullptr, 0 };
            }

        case TargetPlatform::kLinuxX64:
            if (size == 102400) {
                return { kResource_runner_linux_x64_102400_bz2, kResource_runner_linux_x64_102400_bz2_len };
            } else if (size == 524288) {
                return { kResource_runner_linux_x64_524288_bsdiff, kResource_runner_linux_x64_524288_bsdiff_len };
            } else if (size == 1048576) {
                return { kResource_runner_linux_x64_1048576_bsdiff, kResource_runner_linux_x64_1048576_bsdiff_len };
            } else if (size == 5242880) {
                return { kResource_runner_linux_x64_5242880_bsdiff, kResource_runner_linux_x64_5242880_bsdiff_len };
            } else {
                assert(false);
                return { nullptr, 0 };
            }

        case TargetPlatform::kMacX64:
            if (size == 102400) {
                return { kResource_runner_mac_x64_102400_bz2, kResource_runner_mac_x64_102400_bz2_len };
            } else if (size == 524288) {
                return { kResource_runner_mac_x64_524288_bsdiff, kResource_runner_mac_x64_524288_bsdiff_len };
            } else if (size == 1048576) {
                return { kResource_runner_mac_x64_1048576_bsdiff, kResource_runner_mac_x64_1048576_bsdiff_len };
            } else if (size == 5242880) {
                return { kResource_runner_mac_x64_5242880_bsdiff, kResource_runner_mac_x64_5242880_bsdiff_len };
            } else {
                assert(false);
                return { nullptr, 0 };
            }

        case TargetPlatform::kMacArm64:
            if (size == 102400) {
                return { kResource_runner_mac_arm64_102400_bz2, kResource_runner_mac_arm64_102400_bz2_len };
            } else if (size == 524288) {
                return { kResource_runner_mac_arm64_524288_bsdiff, kResource_runner_mac_arm64_524288_bsdiff_len };
            } else if (size == 1048576) {
                return { kResource_runner_mac_arm64_1048576_bsdiff, kResource_runner_mac_arm64_1048576_bsdiff_len };
            } else if (size == 5242880) {
                return { kResource_runner_mac_arm64_5242880_bsdiff, kResource_runner_mac_arm64_5242880_bsdiff_len };
            } else {
                assert(false);
                return { nullptr, 0 };
            }

        case TargetPlatform::kWinX86:
            if (size == 102400) {
                return { kResource_runner_win_x86_102400_bz2, kResource_runner_win_x86_102400_bz2_len };
            } else if (size == 524288) {
                return { kResource_runner_win_x86_524288_bsdiff, kResource_runner_win_x86_524288_bsdiff_len };
            } else if (size == 1048576) {
                return { kResource_runner_win_x86_1048576_bsdiff, kResource_runner_win_x86_1048576_bsdiff_len };
            } else if (size == 5242880) {
                return { kResource_runner_win_x86_5242880_bsdiff, kResource_runner_win_x86_5242880_bsdiff_len };
            } else {
                assert(false);
                return { nullptr, 0 };
            }

        case TargetPlatform::kWinX64:
            if (size == 102400) {
                return { kResource_runner_win_x64_102400_bz2, kResource_runner_win_x64_102400_bz2_len };
            } else if (size == 524288) {
                return { kResource_runner_win_x64_524288_bsdiff, kResource_runner_win_x64_524288_bsdiff_len };
            } else if (size == 1048576) {
                return { kResource_runner_win_x64_1048576_bsdiff, kResource_runner_win_x64_1048576_bsdiff_len };
            } else if (size == 5242880) {
                return { kResource_runner_win_x64_5242880_bsdiff, kResource_runner_win_x64_5242880_bsdiff_len };
            } else {
                assert(false);
                return { nullptr, 0 };
            }

        default:
            assert(false);
            return { nullptr, 0 };
    }
}

static vector<uint8_t> getRunnerTemplate(TargetPlatform platform, size_t bytecodeSize) {
    auto runner102400_bz2 = getRunnerCompressedData(platform, 102400);
    auto runner102400 = decompressBz2(runner102400_bz2.data, runner102400_bz2.len);
    if (bytecodeSize <= 102400) {
        return runner102400;
    }

    ResourceData runner_bsdiff{};
    if (bytecodeSize <= 524288) {
        runner_bsdiff = getRunnerCompressedData(platform, 524288);
    } else if (bytecodeSize <= 1048576) {
        runner_bsdiff = getRunnerCompressedData(platform, 1048576);
    } else if (bytecodeSize <= 5242880) {
        runner_bsdiff = getRunnerCompressedData(platform, 5242880);
    } else {
        throw runtime_error("This program is too big!");
    }

    return patchBsdiff(runner102400.data(), runner102400.size(), runner_bsdiff.data, runner_bsdiff.len);
}

static size_t findBytecodeIndex(const vector<uint8_t>& runnerBinary) {
    // locate the "TTTTT..." block that we will replace with the bytecode
    constexpr size_t sentinelLength = 16;
    auto sentinel = array<char, sentinelLength>();
    memset(sentinel.data(), 'T', sentinelLength);
    for (size_t i = 0; i < runnerBinary.size(); i++) {
        if (runnerBinary.at(i) == 'T' && memcmp(sentinel.data(), &runnerBinary[i], sentinelLength) == 0) {
            return i;
        }
    }

    throw runtime_error("Cannot find the bytecode in the runner binary!");
}

vector<uint8_t> makeExeFile(const vector<uint8_t>& bytecode, TargetPlatform platform) {
    auto runnerBinary = getRunnerTemplate(platform, bytecode.size());
    auto bytecodeIndex = findBytecodeIndex(runnerBinary);
    memcpy(&runnerBinary.at(bytecodeIndex), bytecode.data(), bytecode.size());
    return runnerBinary;
}

}  // namespace compiler
