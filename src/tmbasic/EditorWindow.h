#pragma once

#include "../common.h"
#include "tmbasic/SourceProgram.h"

namespace tmbasic {

class EditorWindow : public TWindow {
   public:
    const unsigned int kBufferSize = 100000;
    EditorWindow(const TRect& r, SourceMember* member, std::function<void()> onEdited);
    void handleEvent(TEvent& event) override;
    uint16_t getHelpCtx() override;
    void close() override;

   private:
    void onTimerTick();
    void updateTitle();

    TEditor* _editor;
    SourceMember* _member;
    std::function<void()> _onEdited;
    int _pendingUpdate = -1;   // -1=no edit pending, 0+=number of ticks since the last edit
    std::string _pendingText;  // text we saw at the last tick
};

}  // namespace tmbasic
