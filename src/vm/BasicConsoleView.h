#pragma once

#include "../common.h"

namespace vm {

class BasicConsoleView : public TView {
   public:
    std::vector<std::vector<TScreenCell>> cells{};
    int16_t currentX{ 0 }, currentY{ 0 };
    TColorAttr currentColorAttr{ TColorRGB{ 255, 255, 255 }, TColorRGB{ 0, 0, 0 } };
    bool isBuffered{ false };

    explicit BasicConsoleView(const TRect& bounds);
    inline TColorAttr mapColor(uchar /*index*/) noexcept override { return 0; }
    void draw() override;
    std::vector<TScreenCell>* getRow(int16_t y);
    TScreenCell* getCell(int16_t x, int16_t y);
    void print(const std::string& str);
};

}  // namespace vm
