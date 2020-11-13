#pragma once

#include "common.h"
#include "shared/vm/Program.h"

namespace tmbasic {

class ProgramItemTypesListBox;
class ProgramContentsListBox;

class ProgramWindow : public TWindow {
   public:
    ProgramWindow(const TRect& r, std::optional<std::string> filePath);
    virtual ~ProgramWindow();
    TPalette& getPalette() const override;
    ushort getHelpCtx() override;
    void handleEvent(TEvent& event) override;
    void close() override;
    bool isDirty();
    bool preClose();

   private:
    bool onSave();
    bool onSaveAs();
    bool save(std::string filePath);
    void setTitleFromFilePath();
    void enableDisableMenuCommands();

    bool _dirty;
    std::optional<std::string> _filePath;
    std::unique_ptr<vm::Program> _program;
    ProgramItemTypesListBox* _typesListBox;
    ProgramContentsListBox* _contentsListBox;
};

}  // namespace tmbasic
