#include "tmbasic/SourceMembersListBox.h"
#include "tmbasic/constants.h"

using compiler::SourceMember;
using compiler::SourceMemberType;
using compiler::SourceProgram;

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
    auto* selectedMember = focused >= 0 && static_cast<size_t>(focused) < _items.size() ? _items[focused] : nullptr;

    _items.clear();

    for (auto& member : _program.members) {
        if (member->memberType == _selectedType) {
            _items.push_back(member.get());
        }
    }

    std::sort(_items.begin(), _items.end(), [](const SourceMember* lhs, const SourceMember* rhs) {
        return lhs->identifier == rhs->identifier ? lhs->displayName < rhs->displayName
                                                  : lhs->identifier < rhs->identifier;
    });

    setRange(_items.size());

    focused = 0;
    for (size_t i = 0; i < _items.size(); i++) {
        if (_items[i] == selectedMember) {
            focused = i;
            break;
        }
    }

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
