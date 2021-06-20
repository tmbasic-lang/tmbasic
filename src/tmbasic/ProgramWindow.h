#pragma once

#include "../common.h"
#include "compiler/SourceProgram.h"
#include "vm/Program.h"

namespace tmbasic {

class ProgramWindow : public TWindow {
   public:
    ProgramWindow(
        const TRect& r,
        std::unique_ptr<compiler::SourceProgram> sourceProgram,
        std::optional<std::string> filePath,
        std::function<void(compiler::SourceMember*)> openMember);
    virtual ~ProgramWindow();
    uint16_t getHelpCtx() override;
    void handleEvent(TEvent& event) override;
    void close() override;
    TPalette& getPalette() const override;
    void setState(uint16_t aState, bool enable) override;
    bool isDirty() const;
    bool preClose();
    void addNewSourceMember(std::unique_ptr<compiler::SourceMember> sourceMember);
    void updateListItems();
    void redrawListItems();
    void checkForErrors();

   private:
    gsl::owner<class ProgramWindowPrivate*> _private;
};

}  // namespace tmbasic
