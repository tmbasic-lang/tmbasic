#include "tmbasic/EditorWindow.h"
#include "../../obj/helpfile.h"
#include "tmbasic/constants.h"

namespace tmbasic {

class EditorIndicator : public TIndicator {
   public:
    explicit EditorIndicator(const TRect& r) : TIndicator(r) {}

    void draw() override {
        auto oldModified = modified;
        modified = false;
        TIndicator::draw();
        modified = oldModified;
    }
};

static std::string getEditorWindowTitle(const SourceMember& member) {
    std::ostringstream s;
    s << member.displayName << " (";
    switch (member.memberType) {
        case SourceMemberType::kConstant:
            s << "constant";
            break;
        case SourceMemberType::kGlobalVariable:
            s << "global";
            break;
        case SourceMemberType::kType:
            s << "type";
            break;
        case SourceMemberType::kProcedure:
            s << "procedure";
            break;
        default:
            assert(false);
            break;
    }
    s << ")";
    return s.str();
}

EditorWindow::EditorWindow(const TRect& r, SourceMember* member)
    : TWindow(r, getEditorWindowTitle(*member), wnNoNumber), TWindowInit(TWindow::initFrame), _member(member) {
    options |= ofTileable;

    auto* hScrollBar = new TScrollBar(TRect(18, size.y - 1, size.x - 2, size.y));
    insert(hScrollBar);

    auto* vScrollBar = new TScrollBar(TRect(size.x - 1, 1, size.x, size.y - 1));
    insert(vScrollBar);

    auto* indicator = new EditorIndicator(TRect(2, size.y - 1, 16, size.y));
    insert(indicator);

    auto editorRect = TRect(getExtent());
    editorRect.grow(-1, -1);
    auto* editor = new TEditor(editorRect, hScrollBar, vScrollBar, indicator, kBufferSize);
    editor->modified = false;

    editor->insertText(member->source.c_str(), member->source.size(), false);
    editor->setSelect(member->selectionStart, member->selectionEnd, true);
    editor->trackCursor(false);
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
