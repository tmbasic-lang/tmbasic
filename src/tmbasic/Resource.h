#pragma once

#include "../common.h"

namespace tmbasic {

enum class Resource {
    kHelp,
    kRunnerLinuxArm32,
    kRunnerLinuxArm64,
    kRunnerLinuxX64,
    kRunnerLinuxX86,
    kRunnerMacX64,
    kRunnerWinX64,
    kRunnerWinX86
};

struct ResourceData {
    uint8_t* start;
    uint8_t* end;
};

ResourceData getResource(Resource resource);

}  // namespace tmbasic
