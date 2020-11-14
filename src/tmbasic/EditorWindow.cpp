#include "EditorWindow.h"
#include "helpfile.h"
#include "constants.h"

namespace tmbasic {

const std::string kInitialSubroutineText("sub untitled()\n\nend sub\n");
const int kInitialSubroutineCursorStart = 4;
const int kInitialSubroutineCursorEnd = 12;
const std::string kInitialFunctionText("function untitled() as integer\n\nend function\n");
const int kInitialFunctionCursorStart = 9;
const int kInitialFunctionCursorEnd = 17;
const std::string kInitialGlobalText("dim untitled as number\n");
const int kInitialGlobalCursorStart = 4;
const int kInitialGlobalCursorEnd = 12;
const std::string kInitialConstantText("const untitled = 1\n");
const int kInitialConstantCursorStart = 6;
const int kInitialConstantCursorEnd = 14;
const std::string kInitialTypeText("type untitled\n\nend type\n");
const int kInitialTypeCursorStart = 5;
const int kInitialTypeCursorEnd = 13;

EditorWindow::EditorWindow(const TRect& r, TextEditorType type)
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

    std::string text;
    int cursorStart, cursorEnd;
    switch (type) {
        case TextEditorType::kSubroutine:
            text = kInitialSubroutineText;
            cursorStart = kInitialSubroutineCursorStart;
            cursorEnd = kInitialSubroutineCursorEnd;
            break;
        case TextEditorType::kFunction:
            text = kInitialFunctionText;
            cursorStart = kInitialFunctionCursorStart;
            cursorEnd = kInitialFunctionCursorEnd;
            break;
        case TextEditorType::kGlobalVariable:
            text = kInitialGlobalText;
            cursorStart = kInitialGlobalCursorStart;
            cursorEnd = kInitialGlobalCursorEnd;
            break;
        case TextEditorType::kConstant:
            text = kInitialConstantText;
            cursorStart = kInitialConstantCursorStart;
            cursorEnd = kInitialConstantCursorEnd;
            break;
        case TextEditorType::kType:
            text = kInitialTypeText;
            cursorStart = kInitialTypeCursorStart;
            cursorEnd = kInitialTypeCursorEnd;
            break;
        default:
            assert(false);
            text = "";
            cursorStart = cursorEnd = 0;
            break;
    }
    editor->insertText(text.c_str(), text.size(), false);
    editor->setSelect(cursorStart, cursorEnd, true);
    insert(editor);
}

void EditorWindow::handleEvent(TEvent& event) {
    TWindow::handleEvent(event);
    if (event.what == evBroadcast) {
        switch (event.message.command) {
            case kCmdCloseProgramRelatedWindows:
                close();
                break;
        }
    }
}

ushort EditorWindow::getHelpCtx() {
    return hcide_editorWindow;
}

}  // namespace tmbasic
