#pragma once

#include "../common.h"
#include "compiler/SourceProgram.h"

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
    bool closeMaybe();
    TPalette& getPalette() const override;
    void setState(uint16_t aState, bool enable) override;
    bool isDirty() const;
    void setDirty();
    bool preClose();
    void addNewSourceMember(std::unique_ptr<compiler::SourceMember> sourceMember);
    void updateListItems();
    void redrawListItems();
    void checkForErrors();
    void publish();
    void run();
    bool save();
    bool saveAs();
    void updateTitle();
    void enableDisableCommands(bool enable);
    void deleteItem();

   private:
    bool _dirty{ false };
    std::optional<std::string> _filePath;
    std::unique_ptr<compiler::SourceProgram> _sourceProgram;
    class SourceMembersListBox* _contentsListBox{};
    std::function<void(compiler::SourceMember*)> _openMember;

    bool save(const std::string& filePath);
};

}  // namespace tmbasic
