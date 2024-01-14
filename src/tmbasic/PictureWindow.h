#pragma once

#include "../common.h"
#include "compiler/SourceProgram.h"
#include "shared/StatusLine.h"

namespace tmbasic {

class PictureWindowStatusItems {
   public:
    shared::StatusLine* statusLine = nullptr;
    TStatusItem* character = nullptr;
    shared::StatusItemColors* characterColor = nullptr;
    TStatusItem* fg = nullptr;
    shared::StatusItemColors* fgColor = nullptr;
    TStatusItem* bg = nullptr;
    shared::StatusItemColors* bgColor = nullptr;
    TStatusItem* select = nullptr;
    shared::StatusItemColors* selectColor = nullptr;
    TStatusItem* draw = nullptr;
    shared::StatusItemColors* drawColor = nullptr;
    TStatusItem* pick = nullptr;
    shared::StatusItemColors* pickColor = nullptr;
    TStatusItem* text = nullptr;
    shared::StatusItemColors* textColor = nullptr;
    TStatusItem* mask = nullptr;
    shared::StatusItemColors* maskColor = nullptr;
    TStatusItem* options = nullptr;
    shared::StatusItemColors* optionsColor = nullptr;
};

class PictureWindow : public TWindow {
   public:
    PictureWindow(
        const TRect& r,
        compiler::SourceMember* member,
        std::function<void()> onEdited,
        const PictureWindowStatusItems& statusItems);
    virtual ~PictureWindow();
    void handleEvent(TEvent& event) override;
    uint16_t getHelpCtx() override;
    void close() override;
    TPalette& getPalette() const override;
    void setState(uint16_t aState, bool enable) override;
    void changeBounds(const TRect& bounds) override;
    void onStatusLineCommand(ushort cmd);

   private:
    gsl::owner<class PictureWindowPrivate*> _private;
};

}  // namespace tmbasic
