#pragma once

#include "../common.h"

namespace tmbasic {

class Button : public TButton {
   public:
    Button(TStringView aTitle, ushort aCommand, ushort aFlags);
};

}  // namespace tmbasic
