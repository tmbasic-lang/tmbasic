#pragma once

#include "../common.h"

namespace compiler {

enum class TargetPlatform {
    kLinuxArm32,
    kLinuxArm64,
    kLinuxX86,
    kLinuxX64,
    kMacArm64,
    kMacX64,
    kWinX86,
    kWinX64,
};

enum class TargetPlatformArchiveType { kZip, kTarGz };

const std::vector<TargetPlatform>& getTargetPlatforms();
const char* getPlatformName(TargetPlatform platform);
TargetPlatform parsePlatformName(const std::string& name);
const char* getPlatformExeExtension(TargetPlatform platform);
TargetPlatformArchiveType getTargetPlatformArchiveType(TargetPlatform platform);
std::string getLicenseForPlatform(TargetPlatform platform);
TargetPlatform getNativeTargetPlatform();

}  // namespace compiler
