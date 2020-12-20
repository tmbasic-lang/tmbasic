#pragma once

#include "../common.h"

namespace tmbasic {

class Label : public TLabel {
   public:
    explicit Label(TStringView text, TView* link = nullptr);
    void draw() override;
};

}  // namespace tmbasic
