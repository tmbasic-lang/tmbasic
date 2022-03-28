#pragma once

#include "../common.h"
#include "compiler/SourceProgram.h"

namespace tmbasic {

class CodeEditorWindow : public turbo::BasicEditorWindow {
   public:
    CodeEditorWindow(
        const TRect& r,
        std::unique_ptr<turbo::Editor> aEditor,
        compiler::SourceMember* member,
        const std::function<void()>& onEdited);
    virtual ~CodeEditorWindow();
    void setState(uint16_t aState, bool enable) override;
    void handleEvent(TEvent& event) override;
    uint16_t getHelpCtx() override;
    void close() override;
    void handleNotification(const SCNotification& scn, turbo::Editor&) override;
    void handleStyleToNeeded(Sci_Position endStyleNeeded);

   private:
    gsl::owner<class CodeEditorWindowPrivate*> _private;
};

}  // namespace tmbasic
