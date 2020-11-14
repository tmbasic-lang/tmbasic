#pragma once

#include "common.h"

namespace tmbasic {

enum class TextEditorType { kFunction, kSubroutine, kGlobalVariable, kConstant, kType };

class EditorWindow : public TWindow {
   public:
    const unsigned int kBufferSize = 100000;
    EditorWindow(const TRect&, TextEditorType type);
    void handleEvent(TEvent& event) override;
    ushort getHelpCtx() override;
};

}  // namespace tmbasic
