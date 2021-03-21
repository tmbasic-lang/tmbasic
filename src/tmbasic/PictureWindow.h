#pragma once

#include "../common.h"
#include "compiler/SourceProgram.h"

namespace tmbasic {

class PictureWindowPrivate;

class PictureWindow : public TWindow {
   public:
    PictureWindow(const TRect& r, compiler::SourceMember* member, std::function<void()> onEdited);
    virtual ~PictureWindow();
    void handleEvent(TEvent& event) override;
    uint16_t getHelpCtx() override;
    void close() override;
    TPalette& getPalette() const override;
    void setState(uint16_t aState, bool enable) override;
    void changeBounds(const TRect& bounds) override;

   private:
    gsl::owner<PictureWindowPrivate*> _private;
};

}  // namespace tmbasic
