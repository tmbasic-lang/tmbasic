#include "SourceMemberTypesListBox.h"
#include "tmbasic/constants.h"

using compiler::SourceMemberType;

namespace tmbasic {

// matches the order of SourceMemberType
static std::vector<std::string> kSourceMemberTypeStrings = { "Procedures", "Globals", "Types", "Designs" };

using SourceMemberTypeSelectedFunc = std::function<void()>;

SourceMemberTypesListBox::SourceMemberTypesListBox(
    const TRect& bounds,
    uint16_t numCols,
    SourceMemberTypeSelectedFunc onSelectedFunc)
    : TListViewer(bounds, numCols, nullptr, nullptr),
      _onSelectedFunc(std::move(onSelectedFunc)),
      _selectedType(SourceMemberType::kProcedure) {
    setRange(4);
}

void SourceMemberTypesListBox::getText(char* dest, int16_t item, int16_t maxLen) {
    strncpy(dest, kSourceMemberTypeStrings[item].c_str(), maxLen);
    dest[maxLen] = '\0';  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

void SourceMemberTypesListBox::focusItem(int16_t item) {
    TListViewer::focusItem(item);
    _selectedType = static_cast<SourceMemberType>(item);
    _onSelectedFunc();
}

SourceMemberType SourceMemberTypesListBox::getSelectedType() const {
    return _selectedType;
}

}  // namespace tmbasic
