#include "PictureWindow.h"
#include "../../obj/resources/help/helpfile.h"
#include "../util/Button.h"
#include "../util/CheckBoxes.h"
#include "../util/clipboard.h"
#include "../util/DialogPtr.h"
#include "../util/InputLine.h"
#include "../util/Label.h"
#include "../util/ScrollBar.h"
#include "../util/StatusLine.h"
#include "../util/ThinButton.h"
#include "../util/ViewPtr.h"
#include "../util/tvutil.h"
#include "../vm/UserForm.h"
#include "App.h"
#include "GridLayout.h"
#include "InsertColorDialog.h"
#include "InsertSymbolDialog.h"
#include "RowLayout.h"
#include "events.h"

using compiler::SourceMember;
using util::Button;
using util::CheckBoxes;
using util::DialogPtr;
using util::InputLine;
using util::Label;
using util::ScrollBar;
using util::StatusLine;
using util::ThinButton;
using util::ViewPtr;

namespace tmbasic {

class ClipboardText {
   public:
    std::string text;
    std::vector<TColorAttr> colors;
    std::vector<uint8_t> mask;  // 0=opaque, 1=transparent
};

// we store a copy of this here because the system clipboard only contains the text characters, but we want to preserve
// colors when copy/pasting within our own app. so track the last copy that we make, and upon pasting, if the system
// clipboard matches this text, then we use these colors.
static ClipboardText _clipboardText{};

class PictureCell {
   public:
    bool transparent = false;
    TColorAttr colorAttr{ TColorRGB{ 0, 0, 0 }, TColorRGB{ 255, 255, 255 } };
    std::string ch = " ";
};

class Picture {
   public:
    std::string name = "Untitled";
    std::vector<PictureCell> cells;
    int width{};
    int height{};

    Picture(int width, int height) : cells(width * height, PictureCell()), width(width), height(height) {}

    explicit Picture(const std::string& source) {
        std::istringstream s{ source };
        s >> std::hex;
        std::string pictureKeyword;
        std::string sizeSeparator;
        s >> pictureKeyword >> name >> sizeSeparator >> width >> height;
        if (pictureKeyword != "picture" || sizeSeparator != "Z") {
            throw std::runtime_error("Unexpected data in picture source.");
        }
        cells = { static_cast<size_t>(width * height), PictureCell() };
        uint32_t fg = 0;
        uint32_t bg = 0;
        int transparent = 0;
        std::string utf8 = " ";
        for (auto y = 0; y < height; y++) {
            for (auto x = 0; x < width; x++) {
                std::string command;
                s >> command;

                auto changesBitMask = command.at(0) - 'A';
                auto charChanged = (changesBitMask & 0x01) != 0;
                auto transparentChanged = (changesBitMask & 0x02) != 0;
                auto fgChanged = (changesBitMask & 0x04) != 0;
                auto bgChanged = (changesBitMask & 0x08) != 0;

                if (charChanged) {
                    std::string utf8Hex;
                    s >> utf8Hex;
                    if ((utf8Hex.size() % 2) != 0) {
                        throw std::runtime_error("Unexpected data in picture source.");
                    }
                    utf8 = "";
                    for (size_t i = 0; i < utf8Hex.size(); i += 2) {
                        auto ch = parseHexByte(utf8Hex.at(i), utf8Hex.at(i + 1));
                        utf8 += ch;
                    }
                }
                if (transparentChanged) {
                    s >> transparent;
                }
                if (fgChanged) {
                    s >> fg;
                }
                if (bgChanged) {
                    s >> bg;
                }

                auto& cell = cells.at(y * width + x);
                cell.transparent = transparent != 0;
                cell.colorAttr = { TColorRGB(fg), TColorRGB(bg) };
                cell.ch = utf8;
            }
        }
    }

    std::string exportToString() {
        std::ostringstream s;
        s << "picture " << name << "\n";
        auto lineStart = s.tellp();
        s << "Z " << std::hex << width << " " << height;
        uint32_t previousFg = 0;
        uint32_t previousBg = 0;
        auto previousTransparent = false;
        std::string previousChar = " ";
        std::function<void()> newlineOrSpace = [&s, &lineStart]() -> void {
            if (s.tellp() - lineStart >= 110) {
                s << "\n";
                lineStart = s.tellp();
            } else {
                s << " ";
            }
        };
        for (auto y = 0; y < height; y++) {
            for (auto x = 0; x < width; x++) {
                auto n = y * width + x;
                auto& cell = cells.at(n);
                auto fg = static_cast<uint32_t>(getFore(cell.colorAttr).asRGB());
                auto bg = static_cast<uint32_t>(getBack(cell.colorAttr).asRGB());

                newlineOrSpace();

                auto changesBitMask = (previousChar != cell.ch ? 0x01 : 0) |
                    (previousTransparent != cell.transparent ? 0x02 : 0) | (previousFg != fg ? 0x04 : 0) |
                    (previousBg != bg ? 0x08 : 0);
                char command = 'A' + changesBitMask;

                s << command;

                if (previousChar != cell.ch) {
                    newlineOrSpace();
                    for (auto ch : cell.ch) {
                        s << std::setw(2) << std::setfill('0') << static_cast<int>(ch & 0xFF);
                    }
                }

                if (previousTransparent != cell.transparent) {
                    newlineOrSpace();
                    s << (cell.transparent ? 1 : 0);
                }

                if (previousFg != fg) {
                    newlineOrSpace();
                    s << fg;
                }

                if (previousBg != bg) {
                    newlineOrSpace();
                    s << bg;
                }

                previousChar = cell.ch;
                previousTransparent = cell.transparent;
                previousFg = fg;
                previousBg = bg;
            }
        }
        s << "\nend picture\n";
        return s.str();
    }

    void resize(int newWidth, int newHeight) {
        if (newWidth < 1) {
            newWidth = 1;
        }
        if (newHeight < 1) {
            newHeight = 1;
        }
        std::vector<PictureCell> newCells{ static_cast<size_t>(newWidth * newHeight), PictureCell() };
        auto commonWidth = std::min(width, newWidth);
        auto commonHeight = std::min(height, newHeight);
        for (auto x = 0; x < commonWidth; x++) {
            for (auto y = 0; y < commonHeight; y++) {
                newCells.at(y * newWidth + x) = cells.at(y * width + x);
            }
        }
        width = newWidth;
        height = newHeight;
        cells = std::move(newCells);
    }

   private:
    static char parseHexNibble(char ch) {
        if (ch >= '0' && ch <= '9') {
            return ch - '0';
        }
        if (ch >= 'a' && ch <= 'f') {
            return ch - 'a' + 10;
        }
        if (ch >= 'A' && ch <= 'F') {
            return ch - 'A' + 10;
        }
        return 0;
    }

    static char parseHexByte(char hi, char lo) {
        char value = parseHexNibble(hi);
        value <<= 4;
        value |= parseHexNibble(lo);
        return value;
    }
};

class PictureViewMouseEventArgs {
   public:
    TPoint viewPoint;
    uint8_t buttons;
    bool down;
    bool move;
    bool up;
};

class PictureView : public TView {
   public:
    Picture picture{ 40, 15 };
    int scrollTop = 0;
    int scrollLeft = 0;

    // select and type modes
    std::optional<TRect> selection;
    bool flashingSelection = false;
    std::optional<Picture> pastedPicture{};
    std::optional<TPoint> pastedPictureLocation{};

    // mask mode
    bool flashingMask = false;

    explicit PictureView(const TRect& bounds) : TView(bounds) {
        eventMask = evMouseDown | evMouseMove | evMouseUp | evCommand | evBroadcast;
        options |= ofSelectable | ofFirstClick;
    }

    int pictureXToView(int pictureX) const { return pictureX - scrollLeft + 2; }

    int pictureYToView(int pictureY) const { return pictureY - scrollTop + 1; }

    int viewXToPicture(int viewX) const { return viewX + scrollLeft - 2; }

    int viewYToPicture(int viewY) const { return viewY + scrollTop - 1; }

    void writeBufferChar(int pictureX, int pictureY, const TDrawBuffer& b) {
        auto viewX = pictureXToView(pictureX);
        auto viewY = pictureYToView(pictureY);
        if (viewX >= 0 && viewX < size.x && viewY >= 0 && viewY < size.y) {
            writeLine(viewX, viewY, 1, 1, b);
        }
    }

    void handleEvent(TEvent& event) override {
        TView::handleEvent(event);
        if (event.what == evMouseDown || event.what == evMouseMove || event.what == evMouseUp) {
            auto lastMouse = makeLocal(event.mouse.where);
            PictureViewMouseEventArgs e{ lastMouse, event.mouse.buttons, event.what == evMouseDown,
                                         event.what == evMouseMove, event.what == evMouseUp };
            message(owner, evCommand, kCmdPictureViewMouse, &e);
            clearEvent(event);
        }
    }

    void drawPictureRegion(const Picture& p, int xViewOffset = 0, int yViewOffset = 0) {
        // p may be the user's picture or it may be a pasted photo being shown overlaid
        for (auto pictureY = 0; pictureY < p.height; pictureY++) {
            TDrawBuffer b;
            auto viewY = pictureYToView(pictureY) + yViewOffset;
            if (viewY < 0 || viewY >= size.y) {
                continue;
            }

            // clip pasted pictures to the document picture bounds
            auto documentPictureY = viewYToPicture(viewY);
            if (documentPictureY < 0 || documentPictureY >= picture.height) {
                continue;
            }

            auto minViewX = size.x;
            auto maxViewX = -1;

            for (auto pictureX = 0; pictureX < p.width; pictureX++) {
                auto viewX = pictureXToView(pictureX) + xViewOffset;
                if (viewX < 0 || viewX >= size.x) {
                    continue;
                }

                // clip pasted pictures to the document picture bounds
                auto documentPictureX = viewXToPicture(viewX);
                if (documentPictureX < 0 || documentPictureX >= picture.width) {
                    continue;
                }

                if (viewX < minViewX) {
                    minViewX = viewX;
                }
                if (viewX > maxViewX) {
                    maxViewX = viewX;
                }

                const auto& cell = p.cells.at(pictureY * p.width + pictureX);
                if (cell.transparent) {
                    if (flashingMask) {
                        b.moveStr(viewX - minViewX, "░", { 0xDF });
                    } else {
                        b.moveStr(viewX - minViewX, "▓", { 0xDF });
                    }
                } else if (!cell.ch.empty()) {
                    b.moveStr(viewX - minViewX, cell.ch, cell.colorAttr);
                } else {
                    b.moveChar(viewX - minViewX, ' ', cell.colorAttr, 1);
                }
            }

            if (maxViewX >= minViewX) {
                writeLine(minViewX, viewY, static_cast<int16_t>(maxViewX - minViewX + 1), 1, b);
            }
        }
    }

    void drawSelectionBorder(
        const Picture& underlyingPicture,
        int x,
        int y,
        int xPictureOffset,
        int yPictureOffset,
        char colors) {
        auto ch = underlyingPicture.cells.at((y - yPictureOffset) * underlyingPicture.width + (x - xPictureOffset)).ch;
        TDrawBuffer b;
        auto flippedColors = ((colors & 0x0F) << 4) | ((colors & 0xF0) >> 4);
        b.moveCStr(0, ch, { flashingSelection ? colors : flippedColors });
        writeBufferChar(x, y, b);
    }

    void drawSelectionRect(
        const Picture& underlyingPicture,
        const TRect& r,
        int xPictureOffset,
        int yPictureOffset,
        char colors) {
        auto left = r.a.x;
        auto right = r.b.x - 1;
        auto top = r.a.y;
        auto bottom = r.b.y - 1;
        for (auto y = r.a.y; y < r.b.y; y++) {
            drawSelectionBorder(underlyingPicture, left, y, xPictureOffset, yPictureOffset, colors);
            drawSelectionBorder(underlyingPicture, right, y, xPictureOffset, yPictureOffset, colors);
        }
        for (auto x = r.a.x; x < r.b.x; x++) {
            drawSelectionBorder(underlyingPicture, x, top, xPictureOffset, yPictureOffset, colors);
            drawSelectionBorder(underlyingPicture, x, bottom, xPictureOffset, yPictureOffset, colors);
        }
    }

    void draw() override {
        {
            TDrawBuffer b;
            b.moveChar(0, ' ', TColorAttr(0x87), size.x);
            for (auto y = 0; y < size.y; y++) {
                writeLine(0, static_cast<int16_t>(y), static_cast<int16_t>(size.x), 1, b);
            }
        }

        drawPictureRegion(picture);

        // show a pasted picture or a selection, not both
        if (pastedPicture.has_value()) {
            auto x = 0;
            auto y = 0;
            if (pastedPictureLocation.has_value()) {
                x = pastedPictureLocation->x;
                y = pastedPictureLocation->y;
            }
            drawPictureRegion(*pastedPicture, x, y);
            drawSelectionRect(
                *pastedPicture, { x, y, x + pastedPicture->width, y + pastedPicture->height }, x, y, 0x3B);
        } else if (selection.has_value()) {
            drawSelectionRect(picture, *selection, 0, 0, 0x2A);
        }

        // top and left grippers are disabled
        {
            TDrawBuffer b;
            b.moveCStr(0, "□", TColorAttr(0x87));
            writeBufferChar(-2, -1, b);
            writeBufferChar(-2, picture.height / 2, b);
            writeBufferChar(-2, picture.height, b);
            writeBufferChar(picture.width / 2, -1, b);
            writeBufferChar(picture.width + 1, -1, b);
        }

        // bottom and right grippers are enabled
        {
            TDrawBuffer b;
            b.moveCStr(0, "■", TColorAttr(0x8F));
            writeBufferChar(picture.width / 2, picture.height, b);
            writeBufferChar(picture.width + 1, picture.height / 2, b);
            writeBufferChar(picture.width + 1, picture.height, b);
        }
    }

    ClipboardText getSelectionTextForClipboard() {
        if (!selection.has_value()) {
            return {};
        }

        auto rect = *selection;
        std::ostringstream text;
        std::vector<TColorAttr> colors;
        std::vector<uint8_t> mask;
        colors.reserve((rect.b.y - rect.a.y) * (rect.b.x - rect.a.x));

        for (auto y = rect.a.y; y < rect.b.y; y++) {
            for (auto x = rect.a.x; x < rect.b.x; x++) {
                const auto& cell = picture.cells.at(y * picture.width + x);
                text << cell.ch;
                colors.push_back(cell.colorAttr);
                mask.push_back(cell.transparent ? 1 : 0);
            }

            // don't end the string with a newline
            if (y < rect.b.y - 1) {
#ifdef _WIN32
                text << "\r";
#endif
                text << "\n";
            }
        }

        return { text.str(), colors, mask };
    }
};

enum class PictureWindowMode {
    // tools
    kSelect,
    kDraw,
    kPick,
    kType,
    kMask,

    // operations
    kPaste
};

class PictureWindowPrivate {
   public:
    int ticks = 0;

    TColorRGB fg{ 255, 255, 255 };
    TColorRGB bg{ 0, 0, 255 };
    std::string ch = "♫";
    PictureWindowMode mode = PictureWindowMode::kDraw;

    // select and type tool
    std::optional<TPoint> currentDrag;

    // select tool > paste operation
    std::optional<TPoint> currentPasteDrag;  // the location where the user started dragging
    std::optional<TPoint>
        currentPasteDragPictureLocation;  // the position of the picture when the user started dragging

    // mask tool
    bool currentDragTransparent = false;

    // drag resizing
    bool resizing = false;
    TPoint resizingStartPicturePoint{};
    std::optional<Picture> resizingOriginalPicture;
    bool resizingVertical = false;
    bool resizingHorizontal = false;

    compiler::SourceMember* member{};
    std::function<void()> onEdited;
    PictureWindowStatusItems statusItems;

    ViewPtr<PictureView> pictureView{ TRect() };
    ViewPtr<ScrollBar> vScrollBar{ TRect(0, 0, 1, 10) };
    ViewPtr<ScrollBar> hScrollBar{ TRect(0, 0, 10, 1) };

    // shared between multiple tools
    ViewPtr<Label> toolLabel{ TRect(1, 1, 10, 2) };
    ViewPtr<CheckBoxes> setFgCheck{ std::vector<std::string>{ "Set FG" }, std::vector<bool>{ true } };
    ViewPtr<CheckBoxes> setBgCheck{ std::vector<std::string>{ "Set BG" }, std::vector<bool>{ true } };
    ViewPtr<CheckBoxes> setChCheck{ std::vector<std::string>{ "Set char" }, std::vector<bool>{ true } };

    // mask tool
    ViewPtr<Label> maskHelp{ "Click to toggle between opaque and transparent." };

    // select tool
    ViewPtr<ThinButton> cutButton{ "Cut", cmCut };
    ViewPtr<ThinButton> copyButton{ "Copy", cmCopy };
    ViewPtr<ThinButton> pasteButton{ "Paste", cmPaste };
    ViewPtr<ThinButton> clearButton{ "Clear", kCmdPictureClear };

    // select tool -> paste operation
    ViewPtr<Label> pasteHelp{ "Drag or use arrow keys to move." };
    ViewPtr<ThinButton> pasteOkButton{ "OK", kCmdPicturePasteOk };
    ViewPtr<ThinButton> pasteCancelButton{ "Cancel", kCmdPicturePasteCancel };
};

class PictureOptionsDialog : public TDialog {
   public:
    explicit PictureOptionsDialog(Picture* picture)
        : TDialog(TRect(0, 0, 0, 0), "Picture Options"),
          TWindowInit(&TDialog::initFrame),
          _picture(picture),
          _nameText(picture->name, 24, 100),
          _widthText(picture->width, 6, 100),
          _heightText(picture->height, 6, 100) {
        options |= ofCentered;

        GridLayout(
            1,
            {
                RowLayout(
                    false,
                    {
                        new Label("~N~ame:", _nameText),
                        _nameText.take(),
                    }),
                RowLayout(
                    false,
                    {
                        new Label("~S~ize:", _widthText),
                        _widthText.take(),
                        new Label("x"),
                        _heightText.take(),
                    }),
                RowLayout(
                    true,
                    {
                        new Button("OK", cmOK, bfDefault),
                        new Button("Cancel", cmCancel, bfNormal),
                    }),
            })
            .addTo(this);

        _nameText->focus();
    }

    void handleEvent(TEvent& event) override {
        if (event.what == evCommand && event.message.command == cmOK) {
            try {
                auto width = util::parseUserInt(_widthText->data, "width", 1, 1000);
                auto height = util::parseUserInt(_heightText->data, "height", 1, 1000);
                util::validateIdentifier(_nameText->data, "name");
                _picture->name = _nameText->data;
                _picture->resize(width, height);
            } catch (std::runtime_error& ex) {
                messageBox(ex.what(), mfError | mfOKButton);
                clearEvent(event);
            }
        }

        TDialog::handleEvent(event);
    }

   private:
    Picture* _picture;
    ViewPtr<InputLine> _nameText;
    ViewPtr<InputLine> _widthText;
    ViewPtr<InputLine> _heightText;
};

static std::string getPictureWindowTitle(const std::string& name) {
    return name + " (Picture)";
}

static void updateScrollBars(PictureWindowPrivate* p) {
    p->vScrollBar->setParams(
        p->vScrollBar->value, 0, std::max(0, p->pictureView->picture.height + 5 - (p->pictureView->size.y - 1)),
        p->pictureView->size.y - 1, 1);
    p->hScrollBar->setParams(
        p->hScrollBar->value, 0, std::max(0, p->pictureView->picture.width + 10 - (p->pictureView->size.x - 1)),
        p->pictureView->size.x - 1, 1);
}

static void showHide(bool showCondition, std::initializer_list<TView*> views) {
    for (auto* view : views) {
        if (showCondition) {
            view->show();
        } else {
            view->hide();
        }
    }
}

static void enableDisableCommands(PictureWindowPrivate* p, bool enable) {
    TCommandSet ts;
    ts.enableCmd(kCmdPictureCharacter);
    ts.enableCmd(kCmdPictureFg);
    ts.enableCmd(kCmdPictureBg);
    ts.enableCmd(kCmdPictureSelect);
    ts.enableCmd(kCmdPictureDraw);
    ts.enableCmd(kCmdPicturePick);
    ts.enableCmd(kCmdPictureType);
    ts.enableCmd(kCmdPictureMask);
    ts.enableCmd(kCmdPictureOptions);
    ts.enableCmd(cmPaste);
    ts.enableCmd(kCmdPictureClear);
    (enable ? TView::enableCommands : TView::disableCommands)(ts);

    TCommandSet tsClipboard;
    tsClipboard.enableCmd(cmCut);
    tsClipboard.enableCmd(cmCopy);
    (enable && p->pictureView->selection.has_value() ? TView::enableCommands : TView::disableCommands)(tsClipboard);
}

static void updateStatusItems(PictureWindowPrivate* p) {
    std::ostringstream chText;
    chText << "~F3~ " << p->ch;
    delete[] p->statusItems.character->text;  // NOLINT
    p->statusItems.character->text = newStr(chText.str());

    TAttrPair attrPair{ TColorAttr(p->fg, p->bg), TColorAttr(p->fg, p->bg) };
    p->statusItems.fgColor->colorPairNormal = attrPair;
    p->statusItems.bgColor->colorPairNormal = attrPair;
    p->statusItems.characterColor->colorPairNormal = attrPair;

    TAttrPair sel{ 0x20, 0x2E };
    TAttrPair unsel{ 0x70, 0x74 };

    p->statusItems.selectColor->colorPairNormal = p->mode == PictureWindowMode::kSelect ? sel : unsel;
    p->statusItems.drawColor->colorPairNormal = p->mode == PictureWindowMode::kDraw ? sel : unsel;
    p->statusItems.pickColor->colorPairNormal = p->mode == PictureWindowMode::kPick ? sel : unsel;
    p->statusItems.textColor->colorPairNormal = p->mode == PictureWindowMode::kType ? sel : unsel;
    p->statusItems.maskColor->colorPairNormal = p->mode == PictureWindowMode::kMask ? sel : unsel;

    StatusLine::setItemText(p->statusItems.select, p->mode == PictureWindowMode::kSelect ? "―► Select" : "~F4~ Select");
    StatusLine::setItemText(p->statusItems.draw, p->mode == PictureWindowMode::kDraw ? "―► Draw" : "~F5~ Draw");
    StatusLine::setItemText(p->statusItems.pick, p->mode == PictureWindowMode::kPick ? "―► Pick" : "~F6~ Pick");
    StatusLine::setItemText(p->statusItems.text, p->mode == PictureWindowMode::kType ? "―► Type" : "~F7~ Type");
    StatusLine::setItemText(p->statusItems.mask, p->mode == PictureWindowMode::kMask ? "―► Mask" : "~F8~ Mask");

    p->statusItems.statusLine->drawView();

    std::string labelText;
    switch (p->mode) {
        case PictureWindowMode::kSelect:
            labelText = "Select";
            break;
        case PictureWindowMode::kDraw:
            labelText = "Draw";
            break;
        case PictureWindowMode::kPick:
            labelText = "Pick";
            break;
        case PictureWindowMode::kType:
            labelText = "Type";
            break;
        case PictureWindowMode::kMask:
            labelText = "Mask";
            break;
        case PictureWindowMode::kPaste:
            labelText = "Paste";
            break;
        default:
            assert(false);
            break;
    }

    // Copy, Move, Fill
    showHide(p->mode == PictureWindowMode::kSelect, { p->copyButton, p->cutButton, p->pasteButton, p->clearButton });

    // Set FG, Set BG
    showHide(
        p->mode == PictureWindowMode::kDraw || p->mode == PictureWindowMode::kType ||
            p->mode == PictureWindowMode::kPick,
        { p->setBgCheck, p->setFgCheck });

    // Set Char
    showHide(p->mode == PictureWindowMode::kDraw || p->mode == PictureWindowMode::kPick, { p->setChCheck });

    // Selection
    if (p->pictureView->selection.has_value()) {
        auto sel = *p->pictureView->selection;
        if (p->mode != PictureWindowMode::kSelect && p->mode != PictureWindowMode::kType) {
            p->pictureView->selection = {};
            enableDisableCommands(p, true);
            p->pictureView->drawView();
        }
        if (p->mode == PictureWindowMode::kType && (sel.b.x > sel.a.x + 1 || sel.b.y > sel.a.y + 1)) {
            p->pictureView->selection = TRect(sel.a.x, sel.a.y, sel.a.x + 1, sel.a.y + 1);
            enableDisableCommands(p, true);
            p->pictureView->drawView();
        }
    }

    // Paste
    showHide(p->mode == PictureWindowMode::kPaste, { p->pasteHelp, p->pasteOkButton, p->pasteCancelButton });

    // Mask help
    showHide(p->mode == PictureWindowMode::kMask, { p->maskHelp });

    p->toolLabel->setTitle(labelText);
    p->toolLabel->drawView();
}

static void onSelectionClear(PictureWindowPrivate* p) {
    PictureCell pictureCell{ false, { p->fg, p->bg }, " " };

    auto& picture = p->pictureView->picture;
    const auto selection = p->pictureView->selection.has_value() ? *p->pictureView->selection
                                                                 : TRect{ 0, 0, picture.width, picture.height };
    for (auto y = selection.a.y; y < selection.b.y; y++) {
        for (auto x = selection.a.x; x < selection.b.x; x++) {
            picture.cells.at(y * picture.width + x) = pictureCell;
        }
    }

    p->pictureView->selection = {};
    enableDisableCommands(p, true);
    p->pictureView->drawView();
}

static void onSelectionCut(PictureWindowPrivate* p) {
    try {
        _clipboardText = p->pictureView->getSelectionTextForClipboard();
        util::setClipboard(_clipboardText.text);
        onSelectionClear(p);
    } catch (std::runtime_error& ex) {
        messageBox(ex.what(), mfError | mfOKButton);
    }
}

static void onSelectionCopy(PictureWindowPrivate* p) {
    try {
        _clipboardText = p->pictureView->getSelectionTextForClipboard();
        util::setClipboard(_clipboardText.text);
    } catch (std::runtime_error& ex) {
        messageBox(ex.what(), mfError | mfOKButton);
    }
}

static void onSelectionPaste(PictureWindowPrivate* p) {
    ClipboardText ct{};
    try {
        ct.text = util::getClipboard();
    } catch (std::runtime_error& ex) {
        messageBox(ex.what(), mfError | mfOKButton);
        return;
    }

    if (ct.text.empty()) {
        return;
    }

    if (ct.text == _clipboardText.text) {
        ct = _clipboardText;
    }

    int x = 0;
    int y = 0;
    int maxLineLength = 0;
    std::vector<std::vector<std::string>> charsByRowThenColumn;
    charsByRowThenColumn.emplace_back();
    for (size_t i = 0; i < ct.text.size();) {
        if (ct.text.at(i) == '\r') {
            i++;
            continue;
        }

        if (ct.text.at(i) == '\n') {
            charsByRowThenColumn.emplace_back();
            x = 0;
            y++;
            i++;
            continue;
        }

        // it may be a multi-byte sequence
        auto len = TText::next(&ct.text.at(i));
        assert(len > 0);
        charsByRowThenColumn.at(y).push_back(ct.text.substr(i, len));
        i += len;
        x++;

        if (x > maxLineLength) {
            maxLineLength = x;
        }
    }

    p->pictureView->pastedPicture = { maxLineLength, static_cast<int>(charsByRowThenColumn.size()) };
    p->pictureView->pastedPictureLocation = { 0, 0 };
    auto& pastedPicture = *p->pictureView->pastedPicture;
    for (int y = 0; y < static_cast<int>(charsByRowThenColumn.size()); y++) {
        const auto& charsByColumn = charsByRowThenColumn.at(y);
        for (int x = 0; x < maxLineLength; x++) {
            auto cellIndex = y * maxLineLength + x;
            auto& cell = pastedPicture.cells.at(cellIndex);
            if (x < static_cast<int>(charsByColumn.size())) {
                cell.ch = charsByColumn.at(x);
            } else {
                cell.transparent = true;
            }
            if (cellIndex < static_cast<int>(ct.colors.size())) {
                cell.colorAttr = ct.colors.at(cellIndex);
            }
            if (cellIndex < static_cast<int>(ct.mask.size())) {
                cell.transparent = ct.mask.at(cellIndex) != 0;
            }
        }
    }

    TView::enableCommand(kCmdPicturePasteOk);
    TView::enableCommand(kCmdPicturePasteCancel);

    p->mode = PictureWindowMode::kPaste;
    updateStatusItems(p);
    p->pictureView->drawView();
}

static void onPasteCancel(PictureWindowPrivate* p) {
    p->currentPasteDrag = {};
    p->currentPasteDragPictureLocation = {};
    p->pictureView->pastedPicture = {};
    p->pictureView->pastedPictureLocation = {};
    p->pictureView->drawView();
}

static void onPasteOk(PictureWindowPrivate* p) {
    const auto& src = *p->pictureView->pastedPicture;
    auto& dst = p->pictureView->picture;
    for (auto ySrc = 0; ySrc < src.height; ySrc++) {
        auto yDst = ySrc + p->pictureView->pastedPictureLocation->y;
        if (yDst < 0 || yDst >= dst.height) {
            continue;
        }
        for (auto xSrc = 0; xSrc < src.width; xSrc++) {
            auto xDst = xSrc + p->pictureView->pastedPictureLocation->x;
            if (xDst < 0 || xDst >= dst.width) {
                continue;
            }
            const auto& cellSrc = src.cells.at(ySrc * src.width + xSrc);
            auto& cellDst = dst.cells.at(yDst * dst.width + xDst);
            cellDst = cellSrc;
        }
    }
    p->pictureView->drawView();
    onPasteCancel(p);
}

PictureWindow::PictureWindow(
    const TRect& r,
    SourceMember* member,
    std::function<void()> onEdited,
    const PictureWindowStatusItems& statusItems)
    : TWindow(r, getPictureWindowTitle(member->identifier), wnNoNumber),
      TWindowInit(TWindow::initFrame),
      _private(new PictureWindowPrivate()) {
    options |= ofTileable;

    _private->member = member;
    _private->onEdited = std::move(onEdited);
    _private->statusItems = statusItems;

    _private->vScrollBar->setBounds(TRect(size.x - 1, 2, size.x, size.y - 1));
    _private->vScrollBar->useWhiteColorScheme();
    _private->vScrollBar.addTo(this);

    _private->hScrollBar->setBounds(TRect(2, size.y - 1, size.x - 2, size.y));
    _private->hScrollBar->useWhiteColorScheme();
    _private->hScrollBar.addTo(this);

    _private->toolLabel.addTo(this);

    RowLayout(
        false,
        {
            _private->setFgCheck.take(),
            _private->setBgCheck.take(),
            _private->setChCheck.take(),
        })
        .addTo(this, 10, 80, 1);
    _private->setFgCheck->hide();
    _private->setBgCheck->hide();
    _private->setChCheck->hide();

    RowLayout(
        false,
        {
            _private->maskHelp.take(),
        })
        .addTo(this, 10, 80, 1);
    _private->maskHelp->hide();

    RowLayout(
        false,
        {
            _private->cutButton.take(),
            _private->copyButton.take(),
            _private->pasteButton.take(),
            _private->clearButton.take(),
        })
        .addTo(this, 10, 80, 1);

    RowLayout(
        false,
        {
            _private->pasteOkButton.take(),
            _private->pasteCancelButton.take(),
            _private->pasteHelp.take(),
        })
        .addTo(this, 10, 80, 1);
    _private->pasteHelp->hide();
    _private->pasteOkButton->hide();
    _private->pasteCancelButton->hide();

    try {
        _private->pictureView->picture = Picture(member->source);
    } catch (...) {
    }

    _private->pictureView->setBounds(TRect(1, 2, size.x - 1, size.y - 1));
    _private->pictureView->growMode = gfGrowHiX | gfGrowHiY;
    _private->pictureView.addTo(this);
    _private->pictureView->select();

    updateScrollBars(_private);
}

PictureWindow::~PictureWindow() {
    delete _private;
}

static void onMouse(int pictureX, int pictureY, const PictureViewMouseEventArgs& e, PictureWindowPrivate* p) {
    TPoint pt{ pictureX, pictureY };
    auto& picture = p->pictureView->picture;
    auto leftMouseDown = e.down && (e.buttons & mbLeftButton) != 0;
    auto rightMouseDown = e.down && (e.buttons & mbRightButton) != 0;
    auto leftDragging = e.move && (e.buttons & mbLeftButton) != 0;
    auto rightDragging = e.move && (e.buttons & mbRightButton) != 0;

    TPoint verticalGripper{ picture.width / 2, picture.height };
    TPoint horizontalGripper{ picture.width + 1, picture.height / 2 };
    TPoint diagonalGripper{ picture.width + 1, picture.height };
    if (leftMouseDown && (pt == verticalGripper || pt == horizontalGripper || pt == diagonalGripper)) {
        // the user has started to drag the resize gripper
        p->resizing = true;
        p->resizingStartPicturePoint = pt;
        p->resizingOriginalPicture = picture;
        p->resizingVertical = pt == verticalGripper || pt == diagonalGripper;
        p->resizingHorizontal = pt == horizontalGripper || pt == diagonalGripper;
        p->pictureView->selection = {};
        enableDisableCommands(p, true);
        p->pictureView->drawView();
        return;
    }
    if (leftDragging && p->resizing) {
        // the user continues to drag the resize gripper
        auto deltaX = 0;
        if (p->resizingHorizontal) {
            deltaX = pt.x - p->resizingStartPicturePoint.x;
        }
        auto deltaY = 0;
        if (p->resizingVertical) {
            deltaY = pt.y - p->resizingStartPicturePoint.y;
        }
        auto newWidth = p->resizingOriginalPicture->width + deltaX;
        auto newHeight = p->resizingOriginalPicture->height + deltaY;
        p->pictureView->picture = *p->resizingOriginalPicture;
        p->pictureView->picture.resize(newWidth, newHeight);
        p->pictureView->selection = {};
        enableDisableCommands(p, true);
        p->pictureView->drawView();
        return;
    }
    if (e.up && p->resizing) {
        // the user has stopped dragging the resize gripper
        p->resizing = false;
        p->resizingOriginalPicture = {};
        updateScrollBars(p);
        return;
    }

    // paste handled earlier here because in this mode, the user may click outside the picture
    if (p->mode == PictureWindowMode::kPaste) {
        if (p->currentPasteDrag.has_value()) {
            // the user is continuing to drag the pasted picture
            auto xOffset = pictureX - p->currentPasteDrag->x;
            auto yOffset = pictureY - p->currentPasteDrag->y;
            auto xNew = p->currentPasteDragPictureLocation->x + xOffset;
            auto yNew = p->currentPasteDragPictureLocation->y + yOffset;
            p->pictureView->pastedPictureLocation = { xNew, yNew };
        } else if (e.down) {
            // if this is inside the pasted picture, then the user has started to drag it
            // if this is outside the pasted picture, then the user is accepting the paste
            auto& loc = p->pictureView->pastedPictureLocation;
            if (pictureX >= loc->x && pictureY >= loc->y && pictureX < loc->x + p->pictureView->pastedPicture->width &&
                pictureY < loc->y + p->pictureView->pastedPicture->height) {
                p->currentPasteDrag = { pictureX, pictureY };
                p->currentPasteDragPictureLocation = p->pictureView->pastedPictureLocation;
            } else {
                onPasteOk(p);
                p->mode = PictureWindowMode::kSelect;
                updateStatusItems(p);
                return;  // don't proceed to interpret this mouse click as the start of a drag selection
            }
        }
        if ((e.up || (e.move && e.buttons == 0)) && p->currentPasteDrag.has_value()) {
            // the user has stopped dragging the pasted picture
            p->currentPasteDrag = {};
            p->currentPasteDragPictureLocation = {};
        }
        p->pictureView->drawView();
    }

    if (pt.x < 0 || pt.x >= picture.width || pt.y < 0 || pt.y >= picture.height) {
        // the user clicked outside the picture
        return;
    }

    auto& cell = picture.cells.at(pictureY * picture.width + pictureX);
    TColorAttr color{ p->fg, p->bg };

    switch (p->mode) {
        case PictureWindowMode::kSelect:
            if (p->currentDrag.has_value()) {
                // the user is continuing to drag-select
                auto x1 = p->currentDrag->x;
                auto y1 = p->currentDrag->y;
                auto x2 = pictureX;
                auto y2 = pictureY;
                if (x1 > x2) {
                    std::swap(x1, x2);
                }
                if (y1 > y2) {
                    std::swap(y1, y2);
                }
                x2++;
                y2++;
                p->pictureView->selection = TRect(x1, y1, x2, y2);
                enableDisableCommands(p, true);
            } else if (e.down) {
                // the user has started drag-selecting
                p->currentDrag = { pictureX, pictureY };
                p->pictureView->selection =
                    TRect(*p->currentDrag, TPoint{ p->currentDrag->x + 1, p->currentDrag->y + 1 });
                enableDisableCommands(p, true);
            }
            if ((e.up || (e.move && e.buttons == 0)) && p->currentDrag.has_value()) {
                // the user has stopped drag-selecting
                p->currentDrag = {};
            }
            p->pictureView->drawView();
            break;

        case PictureWindowMode::kDraw:
            if (leftMouseDown || rightMouseDown || leftDragging || rightDragging) {
                // the user is drawing
                auto right = rightMouseDown || rightDragging;
                if (p->setFgCheck->mark(0)) {
                    cell.colorAttr._fg = right ? color._bg : color._fg;
                    cell.colorAttr._style = color._style;
                }
                if (p->setBgCheck->mark(0)) {
                    cell.colorAttr._bg = right ? color._fg : color._bg;
                }
                if (p->setChCheck->mark(0)) {
                    cell.ch = p->ch;
                }
                cell.transparent = false;
                p->pictureView->drawView();
            }
            break;

        case PictureWindowMode::kPick:
            if (leftMouseDown || rightMouseDown || leftDragging || rightDragging) {
                // the user is picking
                auto right = rightMouseDown || rightDragging;
                if (p->setBgCheck->mark(0)) {
                    p->bg = right ? getFore(cell.colorAttr).asRGB() : getBack(cell.colorAttr).asRGB();
                }
                if (p->setFgCheck->mark(0)) {
                    p->fg = right ? getBack(cell.colorAttr).asRGB() : getFore(cell.colorAttr).asRGB();
                }
                if (p->setChCheck->mark(0)) {
                    p->ch = cell.ch;
                }
                updateStatusItems(p);
            }
            break;

        case PictureWindowMode::kType:
            if (leftMouseDown || leftDragging) {
                // the user is clicking to move the cursor in type mode
                p->pictureView->selection = TRect(pictureX, pictureY, pictureX + 1, pictureY + 1);
                enableDisableCommands(p, true);
                p->pictureView->drawView();
            }
            break;

        case PictureWindowMode::kMask:
            if (leftMouseDown) {
                // the user started drawing in mask mode
                p->currentDragTransparent = !cell.transparent;
            }
            if (leftMouseDown || leftDragging) {
                // the user is continuing to draw in mask mode
                cell.transparent = p->currentDragTransparent;
                p->pictureView->drawView();
            }
            break;

        default:
            break;
    }
}

static void onTick(PictureWindowPrivate* p) {
    // blink the selection rectangle
    // blink the transparent cells if we're in mask mode
    p->ticks++;
    if (p->ticks >= 2) {
        if (p->mode == PictureWindowMode::kMask) {
            p->pictureView->flashingMask = !p->pictureView->flashingMask;
            p->pictureView->drawView();
        } else if (
            p->mode == PictureWindowMode::kSelect || p->mode == PictureWindowMode::kType ||
            p->mode == PictureWindowMode::kPaste) {
            p->pictureView->flashingSelection = !p->pictureView->flashingSelection;
            p->pictureView->drawView();
        }
        p->ticks = 0;
    }
}

static void getArrowKeyDirection(uint16_t keyCode, int* outDeltaX, int* outDeltaY) {
    auto deltaX = 0;
    auto deltaY = 0;
    switch (keyCode) {
        case kbLeft:
            deltaX = -1;
            break;
        case kbRight:
            deltaX = 1;
            break;
        case kbUp:
            deltaY = -1;
            break;
        case kbDown:
            deltaY = 1;
            break;
    }
    *outDeltaX = deltaX;
    *outDeltaY = deltaY;
}

void PictureWindow::handleEvent(TEvent& event) {
    if (event.what == evKeyboard) {
        if (event.keyDown.keyCode == kbEsc && _private->mode == PictureWindowMode::kSelect) {
            // user is pressing ESC to deselect
            _private->pictureView->selection = {};
            enableDisableCommands(_private, true);
            _private->pictureView->drawView();
            clearEvent(event);
        } else if (event.keyDown.keyCode == kbEsc && _private->mode == PictureWindowMode::kPaste) {
            // user is pressing ESC to cancel a paste
            onPasteCancel(_private);
            _private->mode = PictureWindowMode::kSelect;
            updateStatusItems(_private);
            clearEvent(event);
        } else if (event.keyDown.keyCode == kbEnter && _private->mode == PictureWindowMode::kPaste) {
            // user is pressing Enter to confirm a paste
            onPasteOk(_private);
            _private->mode = PictureWindowMode::kSelect;
            updateStatusItems(_private);
            clearEvent(event);
        } else if (
            _private->mode == PictureWindowMode::kType && _private->pictureView->selection.has_value() &&
            event.keyDown.text[0] != '\0' && event.keyDown.keyCode != kbLeft && event.keyDown.keyCode != kbRight &&
            event.keyDown.keyCode != kbUp && event.keyDown.keyCode != kbDown) {
            // user is typing text in type mode
            auto rect = *_private->pictureView->selection;
            auto& cell =
                _private->pictureView->picture.cells.at(rect.a.y * _private->pictureView->picture.width + rect.a.x);
            auto fg = getFore(cell.colorAttr).asRGB();
            auto bg = getBack(cell.colorAttr).asRGB();
            if (_private->setFgCheck->mark(0)) {
                fg = _private->fg;
            }
            if (_private->setBgCheck->mark(0)) {
                bg = _private->bg;
            }
            cell.ch = event.keyDown.text;
            cell.colorAttr = TColorAttr(fg, bg);
            cell.transparent = false;

            if (rect.a.x < _private->pictureView->picture.width - 1) {
                rect.a.x++;
                rect.b.x++;
            }
            _private->pictureView->selection = rect;
            enableDisableCommands(_private, true);
            _private->pictureView->drawView();
            clearEvent(event);
        } else if (
            (_private->mode == PictureWindowMode::kSelect || _private->mode == PictureWindowMode::kType) &&
            (event.keyDown.keyCode == kbLeft || event.keyDown.keyCode == kbRight || event.keyDown.keyCode == kbUp ||
             event.keyDown.keyCode == kbDown) &&
            // user is using the arrow keys to move the cursor in select or type mode
            _private->pictureView->selection.has_value()) {
            auto deltaX = 0;
            auto deltaY = 0;
            getArrowKeyDirection(event.keyDown.keyCode, &deltaX, &deltaY);
            auto width = _private->pictureView->picture.width;
            auto height = _private->pictureView->picture.height;
            auto r = *_private->pictureView->selection;
            r.a.x += deltaX;
            r.b.x += deltaX;
            r.a.y += deltaY;
            r.b.y += deltaY;
            if (r.a.x >= 0 && r.a.x <= width && r.b.x >= 0 && r.b.x <= width && r.a.y >= 0 && r.a.y <= height &&
                r.b.y >= 0 && r.b.y <= height) {
                _private->pictureView->selection = r;
                enableDisableCommands(_private, true);
                _private->pictureView->drawView();
            }
        } else if (
            _private->mode == PictureWindowMode::kPaste &&
            (event.keyDown.keyCode == kbLeft || event.keyDown.keyCode == kbRight || event.keyDown.keyCode == kbUp ||
             event.keyDown.keyCode == kbDown)) {
            // user is using the arrow keys to move the pasted picture in paste mode
            auto deltaX = 0;
            auto deltaY = 0;
            getArrowKeyDirection(event.keyDown.keyCode, &deltaX, &deltaY);
            auto width = _private->pictureView->picture.width;
            auto height = _private->pictureView->picture.height;
            auto p = *_private->pictureView->pastedPictureLocation;
            p.x += deltaX;
            p.y += deltaY;
            if (p.x >= 0 && p.x < width && p.y >= 0 && p.y < height) {
                _private->pictureView->pastedPictureLocation = p;
                _private->pictureView->drawView();
            }
        }
    }

    TWindow::handleEvent(event);

    if (event.what == evCommand) {
        switch (event.message.command) {
            case kCmdPictureViewMouse: {
                auto* e = reinterpret_cast<PictureViewMouseEventArgs*>(event.message.infoPtr);  // NOLINT
                auto pictureX = _private->pictureView->viewXToPicture(e->viewPoint.x);
                auto pictureY = _private->pictureView->viewYToPicture(e->viewPoint.y);
                onMouse(pictureX, pictureY, *e, _private);
                break;
            }

            case kCmdPictureClear:
                onSelectionClear(_private);
                clearEvent(event);
                break;

            case kCmdPicturePasteCancel:
                onPasteCancel(_private);
                _private->mode = PictureWindowMode::kSelect;
                updateStatusItems(_private);
                clearEvent(event);
                break;

            case kCmdPicturePasteOk:
                onPasteOk(_private);
                _private->mode = PictureWindowMode::kSelect;
                updateStatusItems(_private);
                clearEvent(event);
                break;

            case cmCut:
                if (_private->mode == PictureWindowMode::kSelect) {
                    onSelectionCut(_private);
                }
                clearEvent(event);
                break;

            case cmCopy:
                if (_private->mode == PictureWindowMode::kSelect) {
                    onSelectionCopy(_private);
                }
                clearEvent(event);
                break;

            case cmPaste:
                if (_private->mode == PictureWindowMode::kPaste) {
                    onPasteOk(_private);
                }
                onSelectionPaste(_private);
                clearEvent(event);
                break;
        }
    } else if (event.what == evBroadcast) {
        switch (event.message.command) {
            case kCmdCloseProgramRelatedWindows:
                close();
                break;

            case kCmdFindPictureWindow: {
                auto* e = static_cast<FindPictureWindowEventArgs*>(event.message.infoPtr);
                if (e->member == _private->member) {
                    e->window = this;
                    clearEvent(event);
                }
                break;
            }

            case cmScrollBarChanged:
                _private->pictureView->scrollTop = _private->vScrollBar->value;
                _private->pictureView->scrollLeft = _private->hScrollBar->value;
                _private->pictureView->drawView();
                break;

            case kCmdTimerTick:
                onTick(_private);
                break;
        }
    }
}

uint16_t PictureWindow::getHelpCtx() {
    return hcide_pictureWindow;
}

void PictureWindow::close() {
    auto newSource = _private->pictureView->picture.exportToString();
    if (newSource != _private->member->source) {
        _private->member->setSource(newSource);
        _private->onEdited();
    }
    TWindow::close();
}

TPalette& PictureWindow::getPalette() const {
    static auto palette = TPalette(cpGrayDialog, sizeof(cpGrayDialog) - 1);
    return palette;
}

void PictureWindow::setState(uint16_t aState, bool enable) {
    TWindow::setState(aState, enable);

    if (aState == sfActive) {
        if (enable) {
            updateStatusItems(_private);
        }
        dynamic_cast<App&>(*TProgram::application).setPictureWindow(enable ? this : nullptr);

        enableDisableCommands(_private, enable);
    }
}

void PictureWindow::changeBounds(const TRect& bounds) {
    TWindow::changeBounds(bounds);
    updateScrollBars(_private);
}

void PictureWindow::onStatusLineCommand(ushort cmd) {
    auto previousMode = _private->mode;

    if (cmd != kCmdPictureMask) {
        _private->pictureView->flashingMask = false;
        _private->pictureView->drawView();
    }

    switch (cmd) {
        case kCmdPictureFg: {
            TColorRGB selection{};
            if (InsertColorDialog::go("Choose Foreground Color", "Choose", &selection)) {
                _private->fg = selection;
                updateStatusItems(_private);
            }
            break;
        }

        case kCmdPictureBg: {
            TColorRGB selection{};
            if (InsertColorDialog::go("Choose Background Color", "Choose", &selection)) {
                _private->bg = selection;
                updateStatusItems(_private);
            }
            break;
        }

        case kCmdPictureCharacter: {
            auto dialog = DialogPtr<InsertSymbolDialog>("Choose Character", "Choose");
            if (TProgram::deskTop->execView(dialog) == cmOK) {
                _private->ch = dialog->getSelection();
                updateStatusItems(_private);
            }
            break;
        }

        case kCmdPictureSelect:
            _private->mode = PictureWindowMode::kSelect;
            updateStatusItems(_private);
            break;

        case kCmdPictureDraw:
            _private->mode = PictureWindowMode::kDraw;
            updateStatusItems(_private);
            break;

        case kCmdPicturePick:
            _private->mode = PictureWindowMode::kPick;
            updateStatusItems(_private);
            break;

        case kCmdPictureType:
            _private->mode = PictureWindowMode::kType;
            if (!_private->pictureView->selection.has_value() && _private->pictureView->picture.width > 0 &&
                _private->pictureView->picture.height > 0) {
                _private->pictureView->selection = { 0, 0, 1, 1 };
                enableDisableCommands(_private, true);
                _private->pictureView->drawView();
            }
            updateStatusItems(_private);
            break;

        case kCmdPictureMask:
            _private->mode = PictureWindowMode::kMask;
            updateStatusItems(_private);
            break;

        case kCmdPictureOptions: {
            DialogPtr<PictureOptionsDialog> dialog{ &_private->pictureView->picture };
            if (TProgram::deskTop->execView(dialog) == cmOK) {
                _private->pictureView->drawView();
                updateScrollBars(_private);
                delete[] title;
                title = newStr(getPictureWindowTitle(_private->pictureView->picture.name));
                frame->drawView();
                _private->onEdited();
            }
            break;
        }

        default:
            break;
    }

    if (previousMode == PictureWindowMode::kPaste && _private->mode != PictureWindowMode::kPaste) {
        onPasteOk(_private);
    }
}

}  // namespace tmbasic
