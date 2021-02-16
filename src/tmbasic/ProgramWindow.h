#pragma once

#include "../common.h"
#include "compiler/SourceProgram.h"
#include "vm/Program.h"
#include "tmbasic/EditorWindow.h"
#include "tmbasic/SourceMemberTypesListBox.h"
#include "tmbasic/SourceMembersListBox.h"

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
    bool isDirty() const;
    bool preClose();
    void addNewSourceMember(std::unique_ptr<compiler::SourceMember> sourceMember);
    void updateListItems();
    void redrawListItems();

   private:
    bool onSave();
    bool onSaveAs();
    bool save(const std::string& filePath);
    void updateTitle();
    void enableDisableMenuCommands();

    bool _dirty;
    std::optional<std::string> _filePath;
    std::unique_ptr<vm::Program> _vmProgram;
    std::unique_ptr<compiler::SourceProgram> _sourceProgram;
    SourceMemberTypesListBox* _typesListBox;
    SourceMembersListBox* _contentsListBox;
    std::function<void(compiler::SourceMember*)> _openMember;
};

}  // namespace tmbasic
