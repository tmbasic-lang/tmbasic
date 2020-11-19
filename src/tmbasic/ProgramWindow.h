#pragma once

#include "../common.h"
#include "shared/vm/Program.h"
#include "EditorWindow.h"
#include "SourceProgram.h"

namespace tmbasic {

class SourceMemberTypesListBox;
class SourceMembersListBox;

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
    void addNewSourceMember(std::unique_ptr<SourceMember> sourceMember);

   private:
    bool onSave();
    bool onSaveAs();
    bool save(std::string filePath);
    void updateTitle();
    void enableDisableMenuCommands();

    bool _dirty;
    std::optional<std::string> _filePath;
    std::unique_ptr<vm::Program> _vmProgram;
    std::unique_ptr<SourceProgram> _sourceProgram;
    SourceMemberTypesListBox* _typesListBox;
    SourceMembersListBox* _contentsListBox;
};

}  // namespace tmbasic
