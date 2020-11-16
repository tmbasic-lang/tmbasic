#pragma once

#include "common.h"
#include "SourceProgram.h"

namespace tmbasic {

class EditorWindow : public TWindow {
   public:
    const unsigned int kBufferSize = 100000;
    EditorWindow(const TRect&, SourceMember& member);
    void handleEvent(TEvent& event) override;
    ushort getHelpCtx() override;

   private:
    SourceMember& _member;
};

}  // namespace tmbasic
