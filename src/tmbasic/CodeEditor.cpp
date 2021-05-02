#include "CodeEditor.h"
#include "compiler/Token.h"
#include "compiler/TokenKind.h"
#include "compiler/tokenize.h"
#include "util/clipboard.h"

namespace tmbasic {

class CodeEditorPrivate {
   public:
    explicit CodeEditorPrivate(CodeEditor* editor) : editor(editor) {}

    void drawLines(int y, int count, uint linePtr) const {
        // copied from TEditor::drawLines()
        auto b = std::vector<TScreenCell>(static_cast<size_t>(editor->delta.x + editor->size.x), TScreenCell{});
        while (count-- > 0) {
            formatLine(b.data(), linePtr, editor->delta.x + editor->size.x);
            editor->writeBuf(
                0, static_cast<int16_t>(y), static_cast<int16_t>(editor->size.x), 1, &b.at(editor->delta.x));
            linePtr = editor->nextLine(linePtr);
            y++;
        }
    }

    struct ColorRange {
        TColorAttr colorNormal;
        TColorAttr colorKeyword;
        TColorAttr colorString;
        TColorAttr colorComment;
        TColorAttr colorIdentifier;
        uint end;
    };

    std::vector<compiler::TokenKind> tokenizeLine(uint p) const {
        std::ostringstream stream;
        for (; p < editor->bufLen; p++) {
            auto ch = editor->bufChars(p);
            if (ch[0] == '\r' || ch[0] == '\n') {
                break;
            }
            stream << ch[0];
        }
        auto str = stream.str();
        auto tokens = compiler::tokenize(str, compiler::TokenizeType::kFormat);
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

    void formatLine(TScreenCell* drawBuf, uint p, uint width) const {
        auto pStart = p;
        auto tokenKinds = tokenizeLine(p);
        tokenKinds.emplace_back();

        // copied from TEditor::formatLine()
        TColorAttr color{ 0x1F };
        TSpan<TScreenCell> cells(drawBuf, width);
        uint x = 0;
        for (auto range : {
                 ColorRange{ 0x17, 0x1E, 0x1D, 0x1B, 0x1F, editor->selStart },
                 ColorRange{ 0x71, 0x71, 0x71, 0x71, 0x71, editor->selEnd },
                 ColorRange{ 0x17, 0x1E, 0x1D, 0x1B, 0x1F, editor->bufLen },
             }) {
            while (p < range.end) {
                auto pOffset = p - pStart;
                if (pOffset < tokenKinds.size()) {
                    switch (tokenKinds.at(pOffset)) {
                        case compiler::TokenKind::kBooleanLiteral:
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
                        case compiler::TokenKind::kRecord:
                        case compiler::TokenKind::kRethrow:
                        case compiler::TokenKind::kReturn:
                        case compiler::TokenKind::kSelect:
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
                        case compiler::TokenKind::kUntil:
                        case compiler::TokenKind::kWend:
                        case compiler::TokenKind::kWhere:
                        case compiler::TokenKind::kWhile:
                        case compiler::TokenKind::kWith:
                            color = range.colorKeyword;
                            break;

                        case compiler::TokenKind::kStringLiteral:
                            color = range.colorString;
                            break;

                        case compiler::TokenKind::kComment:
                            color = range.colorComment;
                            break;

                        case compiler::TokenKind::kIdentifier:
                            color = range.colorIdentifier;
                            break;

                        default:
                            color = range.colorNormal;
                            break;
                    }
                }

                auto chars = editor->bufChars(p);
                auto ch = chars[0];
                if (ch == '\r' || ch == '\n') {
                    while (x < width) {
                        setCell(cells[x++], ' ', color);
                    }
                    return;
                }
                if (ch == '\t') {
                    if (x < width) {
                        do {
                            setCell(cells[x++], ' ', color);
                        } while (x % 8 != 0 && x < width);
                        ++p;
                    } else {
                        break;
                    }
                } else if (!editor->formatCell(cells, x, chars, p, color)) {
                    break;
                }
            }
        }

        while (x < width) {
            setCell(cells[x++], ' ', color);
        }
    }

    bool clipCopy() const {
        auto offset = editor->bufPtr(editor->selStart);
        auto length = editor->selEnd - editor->selStart;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        util::setClipboard(std::string{ &editor->buffer[offset], length });
        return true;
    }

    void clipCut() const {
        if (clipCopy()) {
            editor->deleteSelect();
        }
    }

    void clipPaste() const {
        auto str = util::getClipboard();
        editor->insertBuffer(str.c_str(), 0, str.size(), true, false);
    }

    CodeEditor* editor;
};

CodeEditor::CodeEditor(
    const TRect& bounds,
    TScrollBar* aHScrollBar,
    TScrollBar* aVScrollBar,
    TIndicator* aIndicator,
    uint aBufSize)
    : TEditor(bounds, aHScrollBar, aVScrollBar, aIndicator, aBufSize), _private(new CodeEditorPrivate(this)) {}

CodeEditor::~CodeEditor() {
    delete _private;
}

void CodeEditor::draw() {
    // copied from TEditor::draw()
    if (drawLine != delta.y) {
        drawPtr = lineMove(drawPtr, delta.y - drawLine);
        drawLine = delta.y;
    }
    _private->drawLines(0, size.y, drawPtr);
}

void CodeEditor::handleEvent(TEvent& event) {
    auto initialCurPos = curPos;

    if (event.what == evCommand) {
        switch (event.message.command) {
            case cmCut:
                _private->clipCut();
                clearEvent(event);
                break;
            case cmCopy:
                _private->clipCopy();
                clearEvent(event);
                break;
            case cmPaste:
                _private->clipPaste();
                clearEvent(event);
                break;
        }
    }

    TEditor::handleEvent(event);

    if (curPos.y != initialCurPos.y) {
        drawView();  // force syntax coloring to be drawn for newly edited lines
    }
}

void CodeEditor::updateCommands() {
    TEditor::updateCommands();
    setCmdState(cmCut, hasSelection());
    setCmdState(cmCopy, hasSelection());
    setCmdState(cmPaste, true);
}

TMenuItem& CodeEditor::initContextMenu(TPoint /*pt*/) {
    return *new TMenuItem("Cu~t~", cmCut, kbCtrlX, hcNoContext, "Ctrl+X") +
        *new TMenuItem("~C~opy", cmCopy, kbCtrlC, hcNoContext, "Ctrl+C") +
        *new TMenuItem("~P~aste", cmPaste, kbCtrlV, hcNoContext, "Ctrl+V");
}

}  // namespace tmbasic
