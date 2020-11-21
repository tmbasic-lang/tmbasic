#pragma once

#include "../common.h"
#include "tmbasic/SourceProgram.h"

namespace tmbasic {

class SourceMembersListBox : public TListViewer {
   public:
    SourceMembersListBox(
        const TRect& bounds,
        uint16_t numCols,
        TScrollBar* vScrollBar,
        const SourceProgram& program,
        std::function<void(SourceMember*)> onMemberOpen);
    virtual ~SourceMembersListBox();
    void selectType(SourceMemberType type);
    void updateItems();
    void getText(char* dest, int16_t item, int16_t maxLen) override;
    void selectItem(int16_t item) override;
    TPalette& getPalette() const override;

   private:
    void openMember(int16_t index);

    const SourceProgram& _program;
    SourceMemberType _selectedType;
    std::vector<SourceMember*> _items;
    std::function<void(SourceMember*)> _onMemberOpen;
};

}  // namespace tmbasic
