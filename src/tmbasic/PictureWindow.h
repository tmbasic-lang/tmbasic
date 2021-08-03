#pragma once

#include "../common.h"
#include "compiler/SourceProgram.h"
#include "util/StatusLine.h"

namespace tmbasic {

class PictureWindowStatusItems {
   public:
    util::StatusLine* statusLine = nullptr;
    TStatusItem* character = nullptr;
    util::StatusItemColors* characterColor = nullptr;
    TStatusItem* fg = nullptr;
    util::StatusItemColors* fgColor = nullptr;
    TStatusItem* bg = nullptr;
    util::StatusItemColors* bgColor = nullptr;
    TStatusItem* select = nullptr;
    util::StatusItemColors* selectColor = nullptr;
    TStatusItem* draw = nullptr;
    util::StatusItemColors* drawColor = nullptr;
    TStatusItem* pick = nullptr;
    util::StatusItemColors* pickColor = nullptr;
    TStatusItem* text = nullptr;
    util::StatusItemColors* textColor = nullptr;
    TStatusItem* mask = nullptr;
    util::StatusItemColors* maskColor = nullptr;
    TStatusItem* options = nullptr;
    util::StatusItemColors* optionsColor = nullptr;
};

class PictureWindow : public TWindow {
   public:
    PictureWindow(
        const TRect& r,
        turbo::Clipboard &clipboard,
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
