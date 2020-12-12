#pragma once

#include "../common.h"

namespace tmbasic {

class Label : public TLabel {
   public:
    Label(TStringView text, TView* link);
    void draw() override;
};

}  // namespace tmbasic
