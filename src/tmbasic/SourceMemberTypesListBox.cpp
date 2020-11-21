#include "SourceMemberTypesListBox.h"
#include "tmbasic/constants.h"

namespace tmbasic {

// matches the order of SourceMemberType
static const char* kSourceMemberTypeStrings[] = {
    "Procedures \x10",
    "Globals    \x10",
    "Constants  \x10",
    "Types      \x10",
};

typedef std::function<void()> SourceMemberTypeSelectedFunc;

SourceMemberTypesListBox::SourceMemberTypesListBox(
    const TRect& bounds,
    uint16_t numCols,
    SourceMemberTypeSelectedFunc onSelectedFunc)
    : TListViewer(bounds, numCols, nullptr, nullptr),
      _onSelectedFunc(onSelectedFunc),
      _selectedType(SourceMemberType::kConstant) {
    setRange(4);
}

SourceMemberTypesListBox::~SourceMemberTypesListBox() {}

void SourceMemberTypesListBox::getText(char* dest, int16_t item, int16_t maxLen) {
    strncpy(dest, kSourceMemberTypeStrings[item], maxLen);
    dest[maxLen] = '\0';
}

void SourceMemberTypesListBox::focusItem(int16_t item) {
    TListViewer::focusItem(item);
    _selectedType = static_cast<SourceMemberType>(item);
    _onSelectedFunc();
}

TPalette& SourceMemberTypesListBox::getPalette() const {
    // Active, Inactive, Focused, Selected, Divider
    static const char bytes[] = kWindowPaletteFrameActive kWindowPaletteFramePassive kWindowPaletteScrollerSelectedText
        kWindowPaletteScrollBarControls kWindowPaletteFrameActive;
    static auto palette = TPalette(bytes, sizeof(bytes) - 1);
    return palette;
}

SourceMemberType SourceMemberTypesListBox::getSelectedType() const {
    return _selectedType;
}

}  // namespace tmbasic
