#include "ProcedureWindow.h"

namespace tmbasic {

const std::string kInitialSubroutineText("sub untitled()\n\nend sub\n");
const int kInitialSubroutineCursorStart = 4;
const int kInitialSubroutineCursorEnd = 12;
const std::string kInitialFunctionText("function untitled() as integer\n\nend function\n");
const int kInitialFunctionCursorStart = 9;
const int kInitialFunctionCursorEnd = 17;

ProcedureWindow::ProcedureWindow(const TRect& r, bool function)
    : TWindow(r, "Untitled (procedure)", wnNoNumber), TWindowInit(TWindow::initFrame) {
    options |= ofTileable;

    auto* hScrollBar = new TScrollBar(TRect(18, size.y - 1, size.x - 2, size.y));
    hScrollBar->hide();
    insert(hScrollBar);

    auto* vScrollBar = new TScrollBar(TRect(size.x - 1, 1, size.x, size.y - 1));
    vScrollBar->hide();
    insert(vScrollBar);

    auto* indicator = new TIndicator(TRect(2, size.y - 1, 16, size.y));
    indicator->hide();
    insert(indicator);

    auto editorRect = TRect(getExtent());
    editorRect.grow(-1, -1);
    auto* editor = new TEditor(editorRect, hScrollBar, vScrollBar, indicator, kBufferSize);
    editor->modified = false;
    auto& text = function ? kInitialFunctionText : kInitialSubroutineText;
    editor->insertText(text.c_str(), text.size(), false);
    editor->setSelect(
        function ? kInitialFunctionCursorStart : kInitialSubroutineCursorStart,
        function ? kInitialFunctionCursorEnd : kInitialSubroutineCursorEnd, true);
    insert(editor);
}

}  // namespace tmbasic
