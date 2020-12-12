#pragma once

#include "../common.h"

namespace tmbasic {

void enableDisableCommand(uint16_t command, bool enable);
size_t lengthWithoutTildes(TStringView text);

}  // namespace tmbasic
