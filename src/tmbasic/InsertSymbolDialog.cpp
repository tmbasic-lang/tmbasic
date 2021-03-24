#include "InsertSymbolDialog.h"
#include "../util/Button.h"
#include "../util/Label.h"
#include "../util/ScrollBar.h"
#include "../util/ViewPtr.h"
#include "RowLayout.h"
#include "events.h"
#include "symbols.h"

// derived from tvision's tvdemo example app

/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

using util::Button;
using util::Label;
using util::ScrollBar;
using util::ViewPtr;

namespace tmbasic {

static constexpr std::array<const char*, 256> _cp437toUtf8 = {
    "",  "☺", "☻", "♥", "♦",  "♣", "♠", "•", "◘", "○", "◙", "♂", "♀",  "♪", "♫", "☼", "►", "◄", "↕", "‼", "¶", "§",
    "▬", "↨", "↑", "↓", "→",  "←", "∟", "↔", "▲", "▼", " ", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+",
    ",", "-", ".", "/", "0",  "1", "2", "3", "4", "5", "6", "7", "8",  "9", ":", ";", "<", "=", ">", "?", "@", "A",
    "B", "C", "D", "E", "F",  "G", "H", "I", "J", "K", "L", "M", "N",  "O", "P", "Q", "R", "S", "T", "U", "V", "W",
    "X", "Y", "Z", "[", "\\", "]", "^", "_", "`", "a", "b", "c", "d",  "e", "f", "g", "h", "i", "j", "k", "l", "m",
    "n", "o", "p", "q", "r",  "s", "t", "u", "v", "w", "x", "y", "z",  "{", "|", "}", "~", "⌂", "Ç", "ü", "é", "â",
    "ä", "à", "å", "ç", "ê",  "ë", "è", "ï", "î", "ì", "Ä", "Å", "É",  "æ", "Æ", "ô", "ö", "ò", "û", "ù", "ÿ", "Ö",
    "Ü", "¢", "£", "¥", "₧",  "ƒ", "á", "í", "ó", "ú", "ñ", "Ñ", "ª",  "º", "¿", "⌐", "¬", "½", "¼", "¡", "«", "»",
    "░", "▒", "▓", "│", "┤",  "╡", "╢", "╖", "╕", "╣", "║", "╗", "╝",  "╜", "╛", "┐", "└", "┴", "┬", "├", "─", "┼",
    "╞", "╟", "╚", "╔", "╩",  "╦", "╠", "═", "╬", "╧", "╨", "╤", "╥",  "╙", "╘", "╒", "╓", "╫", "╪", "┘", "┌", "█",
    "▄", "▌", "▐", "▀", "α",  "ß", "Γ", "π", "Σ", "σ", "µ", "τ", "Φ",  "Θ", "Ω", "δ", "∞", "φ", "ε", "∩", "≡", "±",
    "≥", "≤", "⌠", "⌡", "÷",  "≈", "°", "∙", "·", "√", "ⁿ", "²", "■",  ""
};

static int _unicodeTableScrollTop = 0;

class Cp437Table : public TView {
   public:
    std::function<void(std::string)> setCharFunc;

    explicit Cp437Table(const TRect& r) : TView(r) { eventMask |= evKeyboard; }

    void draw() override {
        TDrawBuffer buf;
        auto color = TColorAttr(0x3F);

        for (ushort y = 0; y <= size.y - 1; y++) {
            buf.moveChar(0, ' ', color, static_cast<int16_t>(size.x));
            for (ushort x = 0; x <= size.x - 1; x++) {
                buf.moveChar(x, static_cast<ushort>(32 * y + x), color, static_cast<ushort>(1));
            }
            writeLine(0, y, static_cast<int16_t>(size.x), static_cast<ushort>(1), buf);
        }
        showCursor();
    }

    void handleEvent(TEvent& event) override {
        TView::handleEvent(event);

        if (event.what == evMouseDown) {
            do {
                if (mouseInView(event.mouse.where)) {
                    TPoint spot = makeLocal(event.mouse.where);
                    setCursor(spot.x, spot.y);
                    charFocused();

                    if ((event.mouse.eventFlags & meDoubleClick) != 0) {
                        TEvent okEvent = { 0 };
                        okEvent.what = evCommand;
                        okEvent.message.command = cmOK;
                        putEvent(okEvent);
                        clearEvent(event);
                        return;
                    }
                }
            } while (mouseEvent(event, evMouseMove));
            clearEvent(event);
        } else {
            if (event.what == evKeyboard) {
                switch (event.keyDown.keyCode) {
                    case kbHome:
                        setCursor(0, 0);
                        break;
                    case kbEnd:
                        setCursor(size.x - 1, size.y - 1);
                        break;
                    case kbUp:
                        if (cursor.y > 0) {
                            setCursor(cursor.x, cursor.y - 1);
                        }
                        break;
                    case kbDown:
                        if (cursor.y < size.y - 1) {
                            setCursor(cursor.x, cursor.y + 1);
                        }
                        break;
                    case kbLeft:
                        if (cursor.x > 0) {
                            setCursor(cursor.x - 1, cursor.y);
                        }
                        break;
                    case kbRight:
                        if (cursor.x < size.x - 1) {
                            setCursor(cursor.x + 1, cursor.y);
                        }
                        break;
                    default:
                        setCursor(event.keyDown.charScan.charCode % 32, event.keyDown.charScan.charCode / 32);
                        break;
                }
                charFocused();
                clearEvent(event);
            }
        }
    }

    void charFocused() { setCharFunc(_cp437toUtf8.at(cursor.x + 32 * cursor.y)); }
};

class UnicodeTable : public TView {
   public:
    std::function<void(std::string)> setCharFunc;

    explicit UnicodeTable(const TRect& r) : TView(r) { eventMask |= evKeyboard; }

    void draw() override {
        TDrawBuffer buf;
        auto color = TColorAttr(0x3F);

        for (ushort y = 0; y <= size.y - 1; y++) {
            buf.moveChar(0, ' ', color, static_cast<int16_t>(size.x));
            for (ushort x = 0; x <= size.x - 1; x++) {
                size_t i = 32 * (y + _unicodeTableScrollTop) + x;
                if (i < getSymbolCount()) {
                    buf.moveStr(x, getSymbol(i), color, static_cast<ushort>(1));
                }
            }
            writeLine(0, y, static_cast<int16_t>(size.x), static_cast<ushort>(1), buf);
        }
        showCursor();
    }

    void handleEvent(TEvent& event) override {
        TView::handleEvent(event);

        if (event.what == evMouseDown) {
            do {
                if (mouseInView(event.mouse.where)) {
                    TPoint spot = makeLocal(event.mouse.where);
                    setCursor(spot.x, spot.y);
                    charFocused();

                    if ((event.mouse.eventFlags & meDoubleClick) != 0) {
                        TEvent okEvent = { 0 };
                        okEvent.what = evCommand;
                        okEvent.message.command = cmOK;
                        putEvent(okEvent);
                        clearEvent(event);
                        return;
                    }
                }
            } while (mouseEvent(event, evMouseMove));
            clearEvent(event);
        } else {
            if (event.what == evKeyboard) {
                switch (event.keyDown.keyCode) {
                    case kbHome:
                        setCursor(0, 0);
                        break;
                    case kbEnd:
                        setCursor(size.x - 1, size.y - 1);
                        break;
                    case kbUp:
                        if (cursor.y > 0) {
                            setCursor(cursor.x, cursor.y - 1);
                        }
                        break;
                    case kbDown:
                        if (cursor.y < size.y - 1) {
                            setCursor(cursor.x, cursor.y + 1);
                        }
                        break;
                    case kbLeft:
                        if (cursor.x > 0) {
                            setCursor(cursor.x - 1, cursor.y);
                        }
                        break;
                    case kbRight:
                        if (cursor.x < size.x - 1) {
                            setCursor(cursor.x + 1, cursor.y);
                        }
                        break;
                    default:
                        setCursor(event.keyDown.charScan.charCode % 32, event.keyDown.charScan.charCode / 32);
                        break;
                }
                charFocused();
                clearEvent(event);
            }
        }
    }

    void charFocused() {
        size_t i = cursor.x + 32 * (_unicodeTableScrollTop + cursor.y);
        if (i < getSymbolCount()) {
            setCharFunc(getSymbol(i));
        }
    }
};

class InsertSymbolDialogPrivate {
   public:
    std::string selection;
    ViewPtr<Label> selectionLabel{ TRect(2, 10, 33, 11) };
    ViewPtr<Cp437Table> cp437Table{ TRect(1, 1, 33, 9) };
    ViewPtr<ScrollBar> vScrollBar{ TRect(66, 1, 67, 9) };
    ViewPtr<UnicodeTable> unicodeTable{ TRect(34, 1, 66, 9) };
};

static void updateSelectionLabel(InsertSymbolDialogPrivate* p) {
    if (p->selection.size() == 0) {
        p->selectionLabel->setTitle("No selection");
    } else {
        p->selectionLabel->setTitle(std::string("Selected: ") + p->selection);
    }
    p->selectionLabel->drawView();
}

InsertSymbolDialog::InsertSymbolDialog(const std::string& title, const std::string& insertButtonText)
    : TDialog(TRect(0, 0, 67, 13), title), TWindowInit(&TDialog::initFrame), _private(new InsertSymbolDialogPrivate()) {
    options |= ofCentered;

    _private->selectionLabel.addTo(this);
    updateSelectionLabel(_private);

    _private->cp437Table->setCharFunc = [this](auto str) {
        _private->selection = str;
        updateSelectionLabel(_private);
    };
    _private->cp437Table->blockCursor();
    _private->cp437Table.addTo(this);
    _private->cp437Table->select();

    _private->vScrollBar->useBlueColorScheme();
    _private->vScrollBar->setParams(
        _unicodeTableScrollTop, 0, getSymbolCount() / 32 - _private->vScrollBar->size.y + 1,
        _private->vScrollBar->size.y - 1, 1);
    _private->vScrollBar.addTo(this);

    _private->unicodeTable->setCharFunc = [this](auto str) {
        _private->selection = str;
        updateSelectionLabel(_private);
    };
    _private->unicodeTable->blockCursor();
    _private->unicodeTable.addTo(this);
    _private->unicodeTable->select();

    auto r = getExtent();
    RowLayout(
        true,
        {
            new Button(insertButtonText, cmOK, bfDefault),
            new Button("Cancel", cmCancel, bfNormal),
        })
        .addTo(this, 1, r.b.x - 3, r.b.y - 3);

    _private->cp437Table->focus();
}

InsertSymbolDialog::~InsertSymbolDialog() {
    delete _private;
}

void InsertSymbolDialog::handleEvent(TEvent& event) {
    TDialog::handleEvent(event);
    if (event.what == evBroadcast) {
        if (event.message.command == cmScrollBarChanged) {
            _unicodeTableScrollTop = _private->vScrollBar->value;
            _private->unicodeTable->drawView();
        }
    }
}

std::string InsertSymbolDialog::getSelection() const {
    return _private->selection;
}

}  // namespace tmbasic
