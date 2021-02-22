#pragma once

#include "../common.h"
#include "compiler/SourceProgram.h"

namespace tmbasic {

class EditorWindow : public TWindow {
   public:
    const unsigned int kBufferSize = 100000;
    EditorWindow(const TRect& r, compiler::SourceMember* member, std::function<void()> onEdited);
    void handleEvent(TEvent& event) override;
    uint16_t getHelpCtx() override;
    void close() override;
    void setState(uint16_t aState, bool enable) override;
    void onEditInsertSymbol();

   private:
    void onTimerTick();
    void updateTitle();

    TEditor* _editor = nullptr;
    compiler::SourceMember* _member;
    std::function<void()> _onEdited;
    int _pendingUpdate = -1;   // -1=no edit pending, 0+=number of ticks since the last edit
    std::string _pendingText;  // text we saw at the last tick
};

}  // namespace tmbasic
