#pragma once

#include "../common.h"

namespace util {

class Label : public TLabel {
   public:
    explicit Label(TStringView text, TView* link = nullptr);
};

}  // namespace util
