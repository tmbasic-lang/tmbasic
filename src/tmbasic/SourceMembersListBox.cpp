#include "tmbasic/SourceMembersListBox.h"
#include "tmbasic/constants.h"

namespace tmbasic {

SourceMembersListBox::SourceMembersListBox(
    const TRect& bounds,
    uint16_t numCols,
    TScrollBar* vScrollBar,
    const SourceProgram& program,
    std::function<void(SourceMember*)> onMemberOpen)
    : TListViewer(bounds, numCols, nullptr, vScrollBar),
      _program(program),
      _selectedType(SourceMemberType::kProcedure),
      _onMemberOpen(onMemberOpen) {
    curCommandSet.enableCmd(cmSave);
    curCommandSet.enableCmd(cmSaveAs);
    selectType(SourceMemberType::kProcedure);
}

SourceMembersListBox::~SourceMembersListBox() {}

void SourceMembersListBox::selectType(SourceMemberType type) {
    _selectedType = type;
    updateItems();
}

void SourceMembersListBox::updateItems() {
    _items.clear();
    for (auto& member : _program.members) {
        if (member->memberType == _selectedType) {
            _items.push_back(member.get());
        }
    }
    setRange(_items.size());
    drawView();
}

void SourceMembersListBox::getText(char* dest, int16_t item, int16_t maxLen) {
    if (item >= 0 && static_cast<size_t>(item) < _items.size()) {
        strncpy(dest, _items[item]->displayName.c_str(), maxLen);
        dest[maxLen] = '\0';
    } else {
        dest[0] = '\0';
    }
}

void SourceMembersListBox::selectItem(int16_t item) {
    openMember(item);
    TListViewer::selectItem(item);
}

TPalette& SourceMembersListBox::getPalette() const {
    // Active, Inactive, Focused, Selected, Divider
    static const char bytes[] = kWindowPaletteFrameActive kWindowPaletteFramePassive kWindowPaletteScrollerSelectedText
        kWindowPaletteScrollBarControls kWindowPaletteFrameActive;
    static auto palette = TPalette(bytes, sizeof(bytes) - 1);
    return palette;
}

void SourceMembersListBox::openMember(int16_t index) {
    if (index >= 0 && static_cast<size_t>(index) < _items.size()) {
        _onMemberOpen(_items[index]);
    }
}
};  // namespace tmbasic
