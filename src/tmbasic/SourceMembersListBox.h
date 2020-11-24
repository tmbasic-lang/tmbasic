#pragma once

#include "../common.h"
#include "compiler/SourceProgram.h"

namespace tmbasic {

class SourceMembersListBox : public TListViewer {
   public:
    SourceMembersListBox(
        const TRect& bounds,
        uint16_t numCols,
        TScrollBar* vScrollBar,
        const compiler::SourceProgram& program,
        std::function<void(compiler::SourceMember*)> onMemberOpen);
    virtual ~SourceMembersListBox();
    void selectType(compiler::SourceMemberType type);
    void updateItems();
    void getText(char* dest, int16_t item, int16_t maxLen) override;
    void selectItem(int16_t item) override;
    TPalette& getPalette() const override;

   private:
    void openMember(int16_t index);

    const compiler::SourceProgram& _program;
    compiler::SourceMemberType _selectedType;
    std::vector<compiler::SourceMember*> _items;
    std::function<void(compiler::SourceMember*)> _onMemberOpen;
};

}  // namespace tmbasic
