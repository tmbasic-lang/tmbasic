#include "Editor.h"

namespace shared {

Editor::Editor(
    const TRect& bounds,
    TScrollBar* aHScrollBar,
    TScrollBar* aVScrollBar,
    TIndicator* aIndicator,
    uint aBufSize)
    : TEditor(bounds, aHScrollBar, aVScrollBar, aIndicator, aBufSize) {}

std::string Editor::getText() const {
    auto prefix = std::string(buffer, curPtr);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto suffix = std::string(buffer + curPtr + gapLen, bufLen - curPtr);
    return prefix + suffix;
}

void Editor::setText(const std::string& str) {
    auto prefixLen = curPtr;
    auto suffixLen = bufLen - curPtr;
    auto len = prefixLen + suffixLen;
    setSelect(0, len, false);
    deleteSelect();
    insertText(str.c_str(), str.size(), false);
}

}  // namespace shared
