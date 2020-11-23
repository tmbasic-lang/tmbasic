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

    TEditor* _editor;
    SourceMember* _member;
    std::function<void()> _onEdited;
};

}  // namespace tmbasic
