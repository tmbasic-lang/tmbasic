#include "InsertSymbolDialog.h"
#include "Button.h"
#include "RowLayout.h"
#include "ViewPtr.h"
#include "events.h"

// derived from tvision's tvdemo example app

/*
 *      Turbo Vision - Version 2.0
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

namespace tmbasic {

const int cmAsciiTableCmdBase = 910;
const int cmCharFocused = 0;

class TTable : public TView {
   public:
    explicit TTable(const TRect& r);
    void draw() override;
    void handleEvent(TEvent& event) override;
    void charFocused();
};

class TReport : public TView {
   public:
    explicit TReport(const TRect& r);
    void draw() override;
    void handleEvent(TEvent& event) override;

   private:
    int asciiChar;
};

TTable::TTable(const TRect& r) : TView(r) {
    eventMask |= evKeyboard;
}

void TTable::draw() {
    TDrawBuffer buf;
    char color = getColor(6);

    for (ushort y = 0; y <= size.y - 1; y++) {
        buf.moveChar(0, ' ', color, static_cast<int16_t>(size.x));
        for (ushort x = 0; x <= size.x - 1; x++) {
            buf.moveChar(x, static_cast<ushort>(32 * y + x), color, static_cast<ushort>(1));
        }
        writeLine(0, y, static_cast<int16_t>(size.x), static_cast<ushort>(1), buf);
    }
    showCursor();
}

//
// cmCharFocused is a offset value (basically the ascii code of the
// current selected character) thus should be added, not or'ed, to
// cmAsciiTableCmdBase.
//

void TTable::charFocused() {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    auto* payload = reinterpret_cast<void*>(static_cast<size_t>(cursor.x + 32 * cursor.y));
    message(owner, evBroadcast, cmAsciiTableCmdBase + cmCharFocused, payload);
}

void TTable::handleEvent(TEvent& event) {
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

TReport::TReport(const TRect& r) : TView(r) {
    asciiChar = 0;
}

void TReport::draw() {
    TDrawBuffer buf;
    char color = getColor(6);
    std::ostringstream statusStr;

    if (asciiChar == 0 || asciiChar == 255) {
        statusStr << std::left << std::setw(32) << "  No character selected";
    } else {
        statusStr << "  Selected: " << static_cast<char>(asciiChar) << "                  ";
    }

    buf.moveStr(0, statusStr.str(), color);
    writeLine(0, 0, 32, 1, buf);
}

void TReport::handleEvent(TEvent& event) {
    TView::handleEvent(event);
    if (event.what == evBroadcast) {
        if (event.message.command == cmAsciiTableCmdBase + cmCharFocused) {
            asciiChar = static_cast<int>(event.message.infoLong);
            drawView();
        }
    }
}

InsertSymbolDialog::InsertSymbolDialog()
    : TDialog(TRect(0, 0, 34, 15), "Insert Symbol"), TWindowInit(&TDialog::initFrame) {
    options |= ofCentered;

    auto report = ViewPtr<TReport>(TRect(1, 10, 33, 11));
    report->eventMask |= evBroadcast;
    report.addTo(this);

    auto table = ViewPtr<TTable>(TRect(1, 1, 33, 9));
    table->options |= ofFramed;
    table->options |= ofSelectable;
    table->blockCursor();
    table.addTo(this);
    table->select();

    auto r = getExtent();
    RowLayout(
        true,
        {
            new Button("Insert", cmOK, bfDefault),
            new Button("Cancel", cmCancel, bfNormal),
        })
        .addTo(this, 1, r.b.x - 3, r.b.y - 3);

    table->focus();
}

static constexpr std::array<const char*, 256> cp437toUtf8 = {
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

void InsertSymbolDialog::handleEvent(TEvent& event) {
    TDialog::handleEvent(event);
    if (event.what == evBroadcast) {
        if (event.message.command == cmAsciiTableCmdBase + cmCharFocused) {
            selection = cp437toUtf8.at(event.message.infoLong);
        }
    }
}

}  // namespace tmbasic
