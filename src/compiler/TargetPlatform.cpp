#include "TargetPlatform.h"

extern char kLicense[];  // NOLINT(modernize-avoid-c-arrays)
extern uint kLicense_len;

namespace compiler {

static const std::vector<TargetPlatform> _targetPlatforms{
    TargetPlatform::kLinuxArm32, TargetPlatform::kLinuxArm64, TargetPlatform::kLinuxX86, TargetPlatform::kLinuxX64,
    TargetPlatform::kMacArm64,   TargetPlatform::kMacX64,     TargetPlatform::kWinX86,   TargetPlatform::kWinX64,
};

const std::vector<TargetPlatform>& getTargetPlatforms() {
    return _targetPlatforms;
}

const char* getPlatformName(TargetPlatform platform) {
    switch (platform) {
        case TargetPlatform::kLinuxArm32:
            return "linux-arm32";
        case TargetPlatform::kLinuxArm64:
            return "linux-arm64";
        case TargetPlatform::kLinuxX86:
            return "linux-x86";
        case TargetPlatform::kLinuxX64:
            return "linux-x64";
        case TargetPlatform::kMacArm64:
            return "mac-arm64";
        case TargetPlatform::kMacX64:
            return "mac-x64";
        case TargetPlatform::kWinX86:
            return "win-x86";
        case TargetPlatform::kWinX64:
            return "win-x64";
        default:
            assert(false);
            return "";
    }
}

TargetPlatformArchiveType getTargetPlatformArchiveType(TargetPlatform platform) {
    switch (platform) {
        case TargetPlatform::kMacArm64:
        case TargetPlatform::kMacX64:
        case TargetPlatform::kWinX86:
        case TargetPlatform::kWinX64:
            return TargetPlatformArchiveType::kZip;

        case TargetPlatform::kLinuxArm32:
        case TargetPlatform::kLinuxArm64:
        case TargetPlatform::kLinuxX86:
        case TargetPlatform::kLinuxX64:
            return TargetPlatformArchiveType::kTarGz;

        default:
            assert(false);
            return {};
    }
}

const char* getPlatformExeExtension(TargetPlatform platform) {
    switch (platform) {
        case TargetPlatform::kWinX86:
        case TargetPlatform::kWinX64:
            return ".exe";
        default:
            return "";
    }
}

std::string getLicenseForPlatform(TargetPlatform platform) {
    std::string_view sv{ kLicense, kLicense_len };
    switch (platform) {
        case TargetPlatform::kWinX86:
        case TargetPlatform::kWinX64: {
            std::ostringstream o;
            for (auto ch : sv) {
                if (ch == '\n') {
                    o << '\r';
                }
                o << ch;
            }
            return o.str();
        }

        default:
            return std::string{ sv };
    }
}

}  // namespace compiler
