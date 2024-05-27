#include "InsertSymbolDialog.h"
#include "../shared/Button.h"
#include "../shared/Label.h"
#include "../shared/ScrollBar.h"
#include "../shared/ViewPtr.h"
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

using shared::Button;
using shared::Label;
using shared::ScrollBar;
using shared::ViewPtr;

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

// Remember the scroll position between dialog instances.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static int _unicodeTableScrollTop = 0;

class Cp437Table : public TView {
   public:
    std::function<void(std::string)> setCharFunc;

    explicit Cp437Table(const TRect& r) : TView(r) { eventMask |= evKeyboard; }

    void draw() override {
        TDrawBuffer buf;
        TColorAttr const color{ 0x30 };

        for (uint16_t y = 0; y <= size.y - 1; y++) {
            buf.moveChar(0, ' ', color, static_cast<int16_t>(size.x));
            for (uint16_t x = 0; x <= size.x - 1; x++) {
                buf.moveChar(x, static_cast<char>(32 * y + x), color, static_cast<uint16_t>(1));
            }
            writeLine(0, static_cast<int16_t>(y), static_cast<int16_t>(size.x), static_cast<uint16_t>(1), buf);
        }
        showCursor();
    }

    void handleEvent(TEvent& event) override {
        TView::handleEvent(event);

        if (event.what == evMouseDown) {
            do {
                if (mouseInView(event.mouse.where)) {
                    TPoint const spot = makeLocal(event.mouse.where);
                    auto i = spot.x + 32 * spot.y;
                    if (i >= 1 && i <= 254) {
                        setCharFunc(_cp437toUtf8.at(i));
                    } else {
                        setCharFunc(" ");
                    }

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
        }
    }
};

class UnicodeTable : public TView {
   public:
    std::function<void(std::string)> setCharFunc;

    explicit UnicodeTable(const TRect& r) : TView(r) { eventMask |= evKeyboard; }

    void draw() override {
        TDrawBuffer buf;
        TColorAttr const color{ 0x30 };

        for (ushort y = 0; y <= size.y - 1; y++) {
            buf.moveChar(0, ' ', color, static_cast<int16_t>(size.x));
            for (ushort x = 0; x <= size.x - 1; x++) {
                size_t const i = 32 * (y + _unicodeTableScrollTop) + x;
                if (i < getSymbolCount()) {
                    buf.moveStr(x, getSymbol(i), color, static_cast<ushort>(1));
                }
            }
            writeLine(0, static_cast<int16_t>(y), static_cast<int16_t>(size.x), static_cast<ushort>(1), buf);
        }
        showCursor();
    }

    void handleEvent(TEvent& event) override {
        TView::handleEvent(event);

        if (event.what == evMouseDown) {
            do {
                if (mouseInView(event.mouse.where)) {
                    TPoint const spot = makeLocal(event.mouse.where);
                    size_t const i = spot.x + 32 * (_unicodeTableScrollTop + spot.y);
                    if (i < getSymbolCount()) {
                        setCharFunc(getSymbol(i));
                    } else {
                        setCharFunc(" ");
                    }

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
        }
    }
};

class InsertSymbolDialogPrivate {
   public:
    std::string selection;
    ViewPtr<Label> selectionLabel{ TRect(2, 11, 33, 12) };
    ViewPtr<Cp437Table> cp437Table{ TRect(3, 2, 36, 10) };
    ViewPtr<UnicodeTable> unicodeTable{ TRect(37, 2, 69, 10) };
    ViewPtr<ScrollBar> vScrollBar{ TRect(69, 2, 70, 10) };
};

static void updateSelectionLabel(InsertSymbolDialogPrivate* p) {
    if (p->selection.empty()) {
        p->selectionLabel->setTitle("No selection");
    } else {
        p->selectionLabel->setTitle(std::string("Selected: ") + p->selection);
    }
    p->selectionLabel->drawView();
}

InsertSymbolDialog::InsertSymbolDialog(const std::string& title, const std::string& insertButtonText)
    : TDialog(TRect(0, 0, 73, 14), title), TWindowInit(&TDialog::initFrame), _private(new InsertSymbolDialogPrivate()) {
    options |= ofCentered;

    _private->selectionLabel.addTo(this);
    updateSelectionLabel(_private);

    _private->cp437Table->setCharFunc = [this](auto str) {
        _private->selection = str;
        updateSelectionLabel(_private);
    };
    _private->cp437Table->blockCursor();
    _private->cp437Table.addTo(this);

    _private->vScrollBar->useBlueColorScheme();
    auto symbolCount = static_cast<int>(getSymbolCount());
    auto numRows = symbolCount / 32 - _private->vScrollBar->size.y;
    if ((symbolCount % 32) != 0) {
        numRows++;
    }
    _private->vScrollBar->setParams(_unicodeTableScrollTop, 0, numRows, _private->vScrollBar->size.y - 1, 1);
    _private->vScrollBar.addTo(this);

    _private->unicodeTable->setCharFunc = [this](auto str) {
        _private->selection = str;
        updateSelectionLabel(_private);
    };
    _private->unicodeTable->blockCursor();
    _private->unicodeTable.addTo(this);

    ViewPtr<Button> insertButton{ insertButtonText, cmOK, bfDefault };

    auto r = getExtent();
    RowLayout(
        true,
        {
            insertButton.take(),
            new Button("Cancel", cmCancel, bfNormal),
        })
        .addTo(this, 1, r.b.x - 3, r.b.y - 3);

    insertButton->focus();
}

InsertSymbolDialog::~InsertSymbolDialog() {
    delete _private;
}

void InsertSymbolDialog::handleEvent(TEvent& event) {
    if (event.what == evCommand && event.message.command == cmOK) {
        if (_private->selection.empty()) {
            messageBox("Please choose a symbol.", mfError | mfOKButton);
            clearEvent(event);
        }
    }
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
