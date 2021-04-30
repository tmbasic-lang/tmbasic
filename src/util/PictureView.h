#pragma once

#include "../common.h"

namespace util {

class PictureCell {
   public:
    bool transparent = false;
    TColorAttr colorAttr{ TColorRGB{ 0, 0, 0 }, TColorRGB{ 255, 255, 255 } };
    std::string ch = " ";
};

class Picture {
   public:
    std::string name = "Untitled";
    std::vector<PictureCell> cells;
    int width{};
    int height{};

    Picture(int width, int height);
    explicit Picture(const std::string& source);
    std::string exportToString();
    void resize(int newWidth, int newHeight);
    Picture crop(TRect bounds);
};

class PictureView : public TView {
   public:
    PictureView(Picture picture, TColorBIOS bg);
    PictureView(const TRect& bounds, Picture picture, TColorBIOS bg);
    void draw() override;

   private:
    Picture _picture;
    TColorBIOS _bg;
};

}  // namespace util
