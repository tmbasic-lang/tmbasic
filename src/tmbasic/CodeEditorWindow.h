#pragma once

#include "../common.h"
#include "compiler/SourceProgram.h"

namespace tmbasic {

class CodeEditorWindow : public BaseEditorWindow {
   public:
    CodeEditorWindow(const TRect& r, compiler::SourceMember* member, const std::function<void()>& onEdited);
    virtual ~CodeEditorWindow();
    void setState(uint16_t aState, bool enable) override;
    void handleEvent(TEvent& event) override;
    uint16_t getHelpCtx() override;
    void close() override;
    void notifyStyleToNeeded(Sci::Position endStyleNeeded) override;

   private:
    gsl::owner<class CodeEditorWindowPrivate*> _private;
};

}  // namespace tmbasic
