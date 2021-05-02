#pragma once

#include "../common.h"

namespace tmbasic {

class CodeEditorPrivate;

class CodeEditor : public TEditor {
   public:
    CodeEditor(
        const TRect& bounds,
        TScrollBar* aHScrollBar,
        TScrollBar* aVScrollBar,
        TIndicator* aIndicator,
        uint aBufSize);
    virtual ~CodeEditor();
    void draw() override;
    void handleEvent(TEvent& event) override;
    void updateCommands() override;
    TMenuItem& initContextMenu(TPoint pt) override;

   private:
    gsl::owner<CodeEditorPrivate*> _private;
};

}  // namespace tmbasic
