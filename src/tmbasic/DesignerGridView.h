#pragma once

#include "../common.h"

namespace tmbasic {

class DesignerGridView : public TView {
   public:
    explicit DesignerGridView(const TRect& bounds);
    void draw() override;
    TPalette& getPalette() const override;
};

}  // namespace tmbasic
