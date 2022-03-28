#include "CodeEditorWindow.h"
#include "../../obj/resources/help/helpfile.h"
#include "../compiler/tokenize.h"
#include "../util/DialogPtr.h"
#include "../util/ViewPtr.h"
#include "InsertColorDialog.h"
#include "InsertSymbolDialog.h"
#include "events.h"

using compiler::SourceMember;
using compiler::SourceMemberType;
using util::DialogPtr;

namespace tmbasic {

const TColorRGB kEditorBackground{ 0x000080 };
const TColorRGB kEditorNormalForeground{ 0xC0C0C0 };
const TColorRGB kEditorStringForeground{ 0xFF8AE2 };
const TColorRGB kEditorKeywordForeground{ 0xFFFF00 };
const TColorRGB kEditorCommentForeground{ 0x00FFFF };
const TColorRGB kEditorIdentifierForeground{ 0xFFFFFF };

static const turbo::ColorScheme _codeEditorColors{
    TColorAttr{ kEditorNormalForeground, kEditorBackground },      // sNormal
    TColorAttr{ kEditorBackground, kEditorNormalForeground },      // sSelection
    TColorAttr{ kEditorNormalForeground, kEditorBackground },      // sWhitespace
    TColorAttr{ kEditorNormalForeground, kEditorBackground },      // sCtrlChar
    TColorAttr{ kEditorNormalForeground, kEditorBackground },      // sLineNums
    TColorAttr{ kEditorKeywordForeground, kEditorBackground },     // sKeyword1
    TColorAttr{ kEditorIdentifierForeground, kEditorBackground },  // sKeyword2
    TColorAttr{ kEditorNormalForeground, kEditorBackground },      // sMisc
    TColorAttr{ kEditorNormalForeground, kEditorBackground },      // sPreprocessor
    TColorAttr{ kEditorNormalForeground, kEditorBackground },      // sOperator
    TColorAttr{ kEditorCommentForeground, kEditorBackground },     // sComment
    TColorAttr{ kEditorStringForeground, kEditorBackground },      // sStringLiteral
    TColorAttr{ kEditorNormalForeground, kEditorBackground },      // sCharLiteral
    TColorAttr{ kEditorNormalForeground, kEditorBackground },      // sNumberLiteral
    TColorAttr{ kEditorNormalForeground, kEditorBackground },      // sEscapeSequence
    TColorAttr{ kEditorNormalForeground, kEditorBackground },      // sError
    TColorAttr{ kEditorKeywordForeground, {}, slBold },            // sBraceMatch
};

static const turbo::WindowColorScheme _codeEditorWindowColors{
    TColorAttr{ TColorBIOS{ 0x7 }, kEditorBackground },  // wndFramePassive
    TColorAttr{ TColorBIOS{ 0xF }, kEditorBackground },  // wndFrameActive
    TColorAttr{ TColorBIOS{ 0xA }, kEditorBackground },  // wndFrameIcon
    TColorAttr{ TColorBIOS{ 0x1 }, TColorBIOS{ 0x3 } },  // wndScrollBarPageArea
    TColorAttr{ TColorBIOS{ 0x1 }, TColorBIOS{ 0x3 } },  // wndScrollBarControls
    TColorAttr{ TColorBIOS{ 0xF }, kEditorBackground },  // wndStaticText
    TColorAttr{ TColorBIOS{ 0x8 }, kEditorBackground },  // wndLabelNormal
    TColorAttr{ TColorBIOS{ 0xF }, kEditorBackground },  // wndLabelSelected
    TColorAttr{ TColorBIOS{ 0x6 }, kEditorBackground },  // wndLabelShortcut
    TColorAttr{ TColorBIOS{ 0x0 }, TColorBIOS{ 0x2 } },  // wndButtonNormal
    TColorAttr{ TColorBIOS{ 0xB }, TColorBIOS{ 0x2 } },  // wndButtonDefault
    TColorAttr{ TColorBIOS{ 0xF }, TColorBIOS{ 0x2 } },  // wndButtonSelected
    TColorAttr{ TColorBIOS{ 0x8 }, TColorBIOS{ 0x7 } },  // wndButtonDisabled
    TColorAttr{ TColorBIOS{ 0xE }, TColorBIOS{ 0x2 } },  // wndButtonShortcut
    TColorAttr{ TColorBIOS{ 0x8 }, TColorBIOS{ 0x1 } },  // wndButtonShadow
    // These can be left empty since we don't use them yet.
    {},  // wndClusterNormal
    {},  // wndClusterSelected
    {},  // wndClusterShortcut
    {},  // wndInputLineNormal
    {},  // wndInputLineSelected
    {},  // wndInputLineArrows
    {},  // wndHistoryArrow
    {},  // wndHistorySides
    {},  // wndHistWinScrollBarPageArea
    {},  // wndHistWinScrollBarControls
    {},  // wndListViewerNormal
    {},  // wndListViewerFocused
    {},  // wndListViewerSelected
    {},  // wndListViewerDivider
    {},  // wndInfoPane
    {},  // wndClusterDisabled
};

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
static const turbo::LexerInfo::StyleMapping _codeStyles[] = {
    { 1, turbo::sKeyword1 },       // keyword
    { 2, turbo::sStringLiteral },  // string
    { 3, turbo::sComment },        // comment
    { 4, turbo::sKeyword2 },       // identifier
};

static const turbo::LexerInfo _codeEditorLexerInfo{
    SCLEX_CONTAINER,
    _codeStyles,  // styles
    {},           // keywords
    {},           // properties
};

class CodeEditorFrame : public turbo::BasicEditorFrame {
   public:
    explicit CodeEditorFrame(TRect r) : BasicEditorFrame(r) {}
    TColorAttr mapColor(uchar index) noexcept override {
        auto attr = BasicEditorFrame::mapColor(index);
        if (getBack(attr) == TColorBIOS(1)) {
            return TColorAttr{ getFore(attr), kEditorBackground };
        }
        return attr;
    }
};

static TFrame* initCodeEditorFrame(TRect r) {
    return new CodeEditorFrame(r);
}

class CodeEditorWindowPrivate {
   public:
    CodeEditorWindow* window{};
    compiler::SourceMember* member{};
    std::function<void()> onEdited;
    int pendingUpdate = -1;   // -1=no edit pending, 0+=number of ticks since the last edit
    std::string pendingText;  // text we saw at the last tick

    std::string getEditorText() const {
        std::ostringstream s;
        size_t bytesLeft = window->editor.callScintilla(SCI_GETTEXT, 0, 0) - 1;
        if (bytesLeft) {
            constexpr size_t blockSize = 1 << 20;
            auto writeSize = std::min(bytesLeft, blockSize);
            std::vector<char> buffer(writeSize, '\0');
            auto bufParam = reinterpret_cast<sptr_t>(buffer.data());
            size_t i = 0;
            do {
                window->editor.callScintilla(SCI_SETTARGETRANGE, i, i + writeSize);
                window->editor.callScintilla(SCI_GETTARGETTEXT, 0U, bufParam);
                s << std::string_view(buffer.data(), writeSize);
                i += writeSize;
                bytesLeft -= writeSize;
                if (bytesLeft < writeSize) {
                    writeSize = bytesLeft;
                }
            } while (bytesLeft > 0);
        }
        return s.str();
    }

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

    void updateTitle() const {
        delete[] window->title;
        window->title = newStr(getEditorWindowTitle(*member));
    }

    void onTimerTick() {
        // TODO: It should be possible to tell whether the text changed by listening
        // to Scintilla notifications. Copying the whole text on every tick is wasteful.
        auto newSource = getEditorText();
        if (newSource != pendingText) {
            pendingUpdate = 0;
            pendingText = newSource;
            onEdited();  // set dirty flag right away
        } else if (pendingUpdate >= 4) {
            // after 1 second of no edits, go ahead and parse the text
            updateSourceMember();
            updateTitle();
            window->frame->drawView();
            pendingUpdate = -1;
        } else if (pendingUpdate >= 0) {
            pendingUpdate++;
        }
    }

    void insertTextAtCursor(const std::string& str) const {
        auto pos = window->editor.callScintilla(SCI_GETCURRENTPOS, 0U, 0U);
        window->editor.callScintilla(SCI_INSERTTEXT, -1, reinterpret_cast<sptr_t>(str.c_str()));
        window->editor.callScintilla(SCI_GOTOPOS, pos + str.size(), 0U);
        window->editor.redraw();
    }

    void onEditInsertSymbol() const {
        DialogPtr<InsertSymbolDialog> dialog{ "Insert Symbol", "Insert" };
        if (TProgram::deskTop->execView(dialog) == cmOK) {
            insertTextAtCursor(dialog->getSelection());
        }
    }

    void onEditInsertColor() const {
        TColorRGB rgb{};
        if (InsertColorDialog::go("Insert Color", "Insert", &rgb)) {
            std::ostringstream s;
            s << "Rgb(" << static_cast<int>(rgb.r) << ", " << static_cast<int>(rgb.g) << ", " << static_cast<int>(rgb.b)
              << ")";
            insertTextAtCursor(s.str());
        }
    }

    void updateSourceMember() const {
        auto newSource = getEditorText();
        if (newSource != member->source) {
            member->setSource(std::move(newSource));
            onEdited();
        }
        member->selectionStart = window->editor.callScintilla(SCI_GETSELECTIONSTART, 0U, 0U);
        member->selectionEnd = window->editor.callScintilla(SCI_GETSELECTIONEND, 0U, 0U);
    }
};

CodeEditorWindow::CodeEditorWindow(
    const TRect& r,
    std::unique_ptr<turbo::Editor> aEditor,
    compiler::SourceMember* member,
    const std::function<void()>& onEdited)
    : TWindowInit(initCodeEditorFrame),
      BasicEditorWindow(r, *aEditor.release()),
      _private(new CodeEditorWindowPrivate()) {
    _private->window = this;
    _private->member = member;
    _private->onEdited = onEdited;
    _private->pendingText = member->source;
    TWindow::setState(sfShadow, true);
    editor.theming.setLexerInfo(&_codeEditorLexerInfo);
    editor.theming.setScheme(&_codeEditorColors);
    editor.theming.apply(editor.scintilla);
    setScheme(&_codeEditorWindowColors);
    editor.callScintilla(SCI_SETSEL, member->selectionStart, member->selectionEnd);
    editor.lineNumbers.setState(true);
    editor.wrapping.setState(true, editor.scintilla);
    editor.autoIndent.setState(true);
    _private->updateTitle();
}

CodeEditorWindow::~CodeEditorWindow() {
    delete _private;
}

void CodeEditorWindow::setState(uint16_t aState, bool enable) {
    BasicEditorWindow::setState(aState, enable);
    if (aState == sfActive) {
        if (state & sfExposed && editor.hScrollBar) {
            editor.hScrollBar->setState(sfVisible, false);  // we always word wrap rather than scroll horizontally
        }

        TCommandSet ts;
        ts.enableCmd(kCmdEditInsertSymbol);
        ts.enableCmd(kCmdEditInsertColor);
        (enable ? enableCommands : disableCommands)(ts);
    }
}

void CodeEditorWindow::handleEvent(TEvent& event) {
    if (event.what == evBroadcast) {
        switch (event.message.command) {
            case kCmdUpdateSourceMembers:
                _private->updateSourceMember();
                break;

            case kCmdCloseProgramRelatedWindows:
                close();
                break;

            case kCmdFindEditorWindow: {
                auto* e = static_cast<FindEditorWindowEventArgs*>(event.message.infoPtr);
                if (e->member == _private->member) {
                    e->window = this;
                    clearEvent(event);
                }
                break;
            }

            case kCmdTimerTick:
                _private->onTimerTick();
                break;
        }
    } else if (event.what == evCommand) {
        switch (event.message.command) {
            case kCmdEditInsertSymbol:
                _private->onEditInsertSymbol();
                clearEvent(event);
                break;

            case kCmdEditInsertColor:
                _private->onEditInsertColor();
                clearEvent(event);
                break;
        }
    }

    BasicEditorWindow::handleEvent(event);
}

uint16_t CodeEditorWindow::getHelpCtx() {
    return hcide_editorWindow;
}

void CodeEditorWindow::close() {
    _private->updateSourceMember();
    TWindow::close();
}

static std::vector<compiler::TokenKind> tokenizeLine(std::string_view str) {
    auto tokens = compiler::tokenize(str, compiler::TokenizeType::kFormat, nullptr);
    std::vector<compiler::TokenKind> vec;
    auto x = 0;
    for (auto& token : tokens) {
        while (x < token.columnIndex) {
            vec.push_back(compiler::TokenKind::kIdentifier);
            x++;
        }
        for (size_t i = 0; i < token.text.size(); i++) {
            vec.push_back(token.type);
            x++;
        }
    }
    return vec;
}

void CodeEditorWindow::handleNotification(const SCNotification& scn, turbo::Editor& aEditor) {
    BasicEditorWindow::handleNotification(scn, aEditor);
    if (scn.nmhdr.code == SCN_STYLENEEDED) {
        {
            handleStyleToNeeded(scn.position);
        }
    }
}

void CodeEditorWindow::handleStyleToNeeded(Sci_Position endStyleNeeded) {
    auto startPos = editor.callScintilla(SCI_GETENDSTYLED, 0, 0);
    auto startLineNumber = editor.callScintilla(SCI_LINEFROMPOSITION, startPos, 0);
    auto endLineNumber = editor.callScintilla(SCI_LINEFROMPOSITION, endStyleNeeded, 0);
    for (auto lineNumber = startLineNumber; lineNumber <= endLineNumber; lineNumber++) {
        auto lineStartPos = editor.callScintilla(SCI_POSITIONFROMLINE, lineNumber, 0);
        auto lineLength = editor.callScintilla(SCI_LINELENGTH, lineNumber, 0);
        std::vector<char> line(lineLength + 1, '\0');
        editor.callScintilla(SCI_GETLINE, lineNumber, reinterpret_cast<sptr_t>(line.data()));
        auto kinds = tokenizeLine(std::string_view(line.data(), lineLength));

        // "For example, with the standard settings of 5 style bits and 3 indicator bits, you would use a mask value
        // of 31 (0x1f) if you were setting text styles and did not want to change the indicators"
        // -- Scintilla docs for SCI_STARTSTYLING
        editor.callScintilla(SCI_STARTSTYLING, lineStartPos, 0x1F);

        for (auto& kind : kinds) {
            auto style = 0;
            switch (kind) {
                case compiler::TokenKind::kAnd:
                case compiler::TokenKind::kAs:
                case compiler::TokenKind::kBoolean:
                case compiler::TokenKind::kBy:
                case compiler::TokenKind::kCase:
                case compiler::TokenKind::kCatch:
                case compiler::TokenKind::kConst:
                case compiler::TokenKind::kContinue:
                case compiler::TokenKind::kDate:
                case compiler::TokenKind::kDateTime:
                case compiler::TokenKind::kDateTimeOffset:
                case compiler::TokenKind::kDim:
                case compiler::TokenKind::kDo:
                case compiler::TokenKind::kEach:
                case compiler::TokenKind::kElse:
                case compiler::TokenKind::kEnd:
                case compiler::TokenKind::kExit:
                case compiler::TokenKind::kFalse:
                case compiler::TokenKind::kFinally:
                case compiler::TokenKind::kFor:
                case compiler::TokenKind::kFrom:
                case compiler::TokenKind::kFunction:
                case compiler::TokenKind::kGroup:
                case compiler::TokenKind::kIf:
                case compiler::TokenKind::kIn:
                case compiler::TokenKind::kInput:
                case compiler::TokenKind::kInto:
                case compiler::TokenKind::kJoin:
                case compiler::TokenKind::kKey:
                case compiler::TokenKind::kList:
                case compiler::TokenKind::kLoop:
                case compiler::TokenKind::kMap:
                case compiler::TokenKind::kMod:
                case compiler::TokenKind::kNext:
                case compiler::TokenKind::kNot:
                case compiler::TokenKind::kNumber:
                case compiler::TokenKind::kOf:
                case compiler::TokenKind::kOn:
                case compiler::TokenKind::kOptional:
                case compiler::TokenKind::kOr:
                case compiler::TokenKind::kPrint:
                case compiler::TokenKind::kRecord:
                case compiler::TokenKind::kRethrow:
                case compiler::TokenKind::kReturn:
                case compiler::TokenKind::kSelect:
                case compiler::TokenKind::kShared:
                case compiler::TokenKind::kStep:
                case compiler::TokenKind::kSub:
                case compiler::TokenKind::kString:
                case compiler::TokenKind::kThen:
                case compiler::TokenKind::kThrow:
                case compiler::TokenKind::kTimeSpan:
                case compiler::TokenKind::kTimeZone:
                case compiler::TokenKind::kTo:
                case compiler::TokenKind::kTrue:
                case compiler::TokenKind::kTry:
                case compiler::TokenKind::kType:
                case compiler::TokenKind::kWend:
                case compiler::TokenKind::kWhere:
                case compiler::TokenKind::kWhile:
                case compiler::TokenKind::kWith:
                case compiler::TokenKind::kYield:
                    style = 1;
                    break;

                case compiler::TokenKind::kStringLiteral:
                    style = 2;
                    break;

                case compiler::TokenKind::kComment:
                    style = 3;
                    break;

                case compiler::TokenKind::kIdentifier:
                    style = 4;
                    break;

                default:
                    style = 0;
                    break;
            }

            editor.callScintilla(SCI_SETSTYLING, 1, style);
        }
    }
}

}  // namespace tmbasic
