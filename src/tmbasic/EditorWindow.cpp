#include "tmbasic/EditorWindow.h"
#include "../../obj/resources/help/helpfile.h"
#include "tmbasic/ViewPtr.h"
#include "tmbasic/events.h"

using compiler::SourceMember;
using compiler::SourceMemberType;

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
    s << member.identifier << " (";
    switch (member.memberType) {
        case SourceMemberType::kGlobal:
            s << "Global";
            break;
        case SourceMemberType::kType:
            s << "Type";
            break;
        case SourceMemberType::kProcedure:
            s << "Procedure";
            break;
        default:
            assert(false);
            break;
    }
    s << ")";
    return s.str();
}

EditorWindow::EditorWindow(const TRect& r, SourceMember* member, std::function<void()> onEdited)
    : TWindow(r, getEditorWindowTitle(*member), wnNoNumber),
      TWindowInit(TWindow::initFrame),
      _member(member),
      _onEdited(std::move(onEdited)) {
    palette = wpBlueWindow;
    options |= ofTileable;

    auto hScrollBar = ViewPtr<TScrollBar>(TRect(18, size.y - 1, size.x - 2, size.y));
    hScrollBar.addTo(this);

    auto vScrollBar = ViewPtr<TScrollBar>(TRect(size.x - 1, 1, size.x, size.y - 1));
    vScrollBar.addTo(this);

    auto indicator = ViewPtr<EditorIndicator>(TRect(2, size.y - 1, 16, size.y));
    indicator.addTo(this);

    auto editorRect = TRect(getExtent());
    editorRect.grow(-1, -1);
    auto editor = ViewPtr<TEditor>(editorRect, hScrollBar, vScrollBar, indicator, kBufferSize);
    _editor = editor;
    _editor->modified = false;

    _editor->insertText(member->source.c_str(), member->source.size(), false);
    _editor->setSelect(member->selectionStart, member->selectionEnd, true);
    _editor->trackCursor(false);
    editor.addTo(this);
}

void EditorWindow::handleEvent(TEvent& event) {
    TWindow::handleEvent(event);
    if (event.what == evBroadcast) {
        switch (event.message.command) {
            case kCmdCloseProgramRelatedWindows:
                close();
                break;

            case kCmdFindEditorWindow: {
                auto* e = static_cast<FindEditorWindowEventArgs*>(event.message.infoPtr);
                if (e->member == _member) {
                    e->window = this;
                    clearEvent(event);
                }
                break;
            }

            case kCmdTimerTick:
                onTimerTick();
                break;
        }
    }
}

uint16_t EditorWindow::getHelpCtx() {
    return hcide_editorWindow;
}

static std::string getEditorText(TEditor* editor) {
    auto prefix = std::string(editor->buffer, editor->curPtr);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto suffix = std::string(editor->buffer + editor->curPtr + editor->gapLen, editor->bufLen - editor->curPtr);
    return prefix + suffix;
}

void EditorWindow::close() {
    auto newSource = getEditorText(_editor);
    if (newSource != _member->source) {
        _member->setSource(newSource);
        _onEdited();
    }
    _member->selectionStart = _editor->selStart;
    _member->selectionEnd = _editor->selEnd;
    TWindow::close();
}

void EditorWindow::onTimerTick() {
    auto newSource = getEditorText(_editor);
    if (newSource != _pendingText) {
        _pendingUpdate = 0;
        _pendingText = newSource;
    } else if (_pendingUpdate >= 4) {
        // after 1 second of no edits, go ahead and parse the text
        _member->setSource(newSource);
        _onEdited();
        updateTitle();
        frame->drawView();
        _pendingUpdate = -1;
    } else {
        _pendingUpdate++;
    }
}

void EditorWindow::updateTitle() {
    delete[] title;
    title = strdup(getEditorWindowTitle(*_member).c_str());
}

}  // namespace tmbasic
