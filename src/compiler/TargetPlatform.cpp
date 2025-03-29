#include "TargetPlatform.h"
#include "shared/process.h"
#include "shared/path.h"

namespace compiler {

static const std::vector<TargetPlatform> _targetPlatforms{
#ifdef NDEBUG
    TargetPlatform::kLinuxArm32, TargetPlatform::kLinuxArm64, TargetPlatform::kLinuxX86, TargetPlatform::kLinuxX64,
    TargetPlatform::kMacArm64,   TargetPlatform::kMacX64,     TargetPlatform::kWinX86,   TargetPlatform::kWinX64,
#else
    getNativeTargetPlatform(),
#endif
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

TargetPlatform parsePlatformName(const std::string& name) {
    if (name == "linux-arm32") {
        return TargetPlatform::kLinuxArm32;
    }
    if (name == "linux-arm64") {
        return TargetPlatform::kLinuxArm64;
    }
    if (name == "linux-x86") {
        return TargetPlatform::kLinuxX86;
    }
    if (name == "linux-x64") {
        return TargetPlatform::kLinuxX64;
    }
    if (name == "mac-arm64") {
        return TargetPlatform::kMacArm64;
    }
    if (name == "mac-x64") {
        return TargetPlatform::kMacX64;
    }
    if (name == "win-x86") {
        return TargetPlatform::kWinX86;
    }
    if (name == "win-x64") {
        return TargetPlatform::kWinX64;
    }
    throw std::runtime_error{ "Unknown platform name: " + name };
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
    auto exeDir = shared::getExecutableDirectoryPath();
    auto licensePath = shared::pathCombine(exeDir, "LICENSE.dat");
    std::ifstream licenseFile{ licensePath, std::ios::in | std::ios::binary };
    if (!licenseFile.is_open()) {
        throw std::runtime_error{ "Failed to open license file: " + licensePath };
    }
    std::string licenseContent{ std::istreambuf_iterator<char>{licenseFile}, {} };

    switch (platform) {
        case TargetPlatform::kWinX86:
        case TargetPlatform::kWinX64: {
            std::ostringstream o;
            for (auto ch : licenseContent) {
                if (ch == '\n') {
                    o << '\r';
                }
                o << ch;
            }
            return o.str();
        }

        default:
            return licenseContent;
    }
}

TargetPlatform getNativeTargetPlatform() {
#if defined(__linux__)
#if defined(__arm__) && !defined(__aarch64__)
    return TargetPlatform::kLinuxArm32;
#elif defined(__aarch64__)
    return TargetPlatform::kLinuxArm64;
#elif defined(__i386__)
    return TargetPlatform::kLinuxX86;
#elif defined(__x86_64__)
    return TargetPlatform::kLinuxX64;
#else
#error "getNativeTargetPlatform(): Unsupported Linux platform."
#endif
#elif defined(__APPLE__) && defined(__MACH__)
#if defined(__aarch64__)
    return TargetPlatform::kMacArm64;
#elif defined(__x86_64__)
    return TargetPlatform::kMacX64;
#else
#error "getNativeTargetPlatform(): Unsupported macOS platform."
#endif
#elif defined(_WIN32)
#if defined(_M_IX86)
    return TargetPlatform::kWinX86;
#elif defined(_M_X64)
    return TargetPlatform::kWinX64;
#else
#error "getNativeTargetPlatform(): Unsupported Windows platform."
#endif
#else
#error "getNativeTargetPlatform(): Unsupported platform."
#endif
}

}  // namespace compiler
