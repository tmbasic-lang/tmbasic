#pragma once

#include "../common.h"
#include "compiler/SourceProgram.h"

namespace tmbasic {

class DesignerWindow : public TWindow {
   public:
    DesignerWindow(const TRect& r, compiler::SourceMember* member, std::function<void()> onEdited);
    void handleEvent(TEvent& event) override;
    uint16_t getHelpCtx() override;
    void close() override;

   private:
    void onTimerTick();
    void updateTitle();

    compiler::SourceMember* _member;
    std::function<void()> _onEdited;
};

}  // namespace tmbasic
