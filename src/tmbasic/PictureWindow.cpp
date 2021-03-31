#include "tmbasic/PictureWindow.h"
#include "../../obj/resources/help/helpfile.h"
#include "../util/Button.h"
#include "../util/CheckBoxes.h"
#include "../util/DialogPtr.h"
#include "../util/InputLine.h"
#include "../util/Label.h"
#include "../util/ScrollBar.h"
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
using util::ThinButton;
using util::ViewPtr;

namespace tmbasic {

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
        std::string utf8Hex;
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
                    s >> utf8Hex;
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
                std::string utf8;
                if ((utf8Hex.size() % 2) != 0) {
                    throw std::runtime_error("Unexpected data in picture source.");
                }
                for (size_t i = 0; i < utf8Hex.size(); i += 2) {
                    auto ch = parseHexByte(utf8Hex.at(i), utf8Hex.at(i + 1));
                    utf8 += ch;
                }
                cell.ch = std::move(utf8);
            }
        }
    }

    std::string exportToString() {
        std::ostringstream s;
        s << "picture " << name << "\nZ " << std::hex << width << " " << height;
        uint32_t previousFg = 0;
        uint32_t previousBg = 0;
        auto previousTransparent = false;
        std::string previousChar = " ";
        auto lineStart = s.tellp();
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
                        s << std::setw(2) << std::setfill('0') << static_cast<int>(ch);
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

    void drawPictureRegion(TRect pictureRect) {
        for (auto pictureY = pictureRect.a.y; pictureY < pictureRect.b.y; pictureY++) {
            TDrawBuffer b;
            auto viewY = pictureYToView(pictureY);
            if (viewY < 0 || viewY >= size.y) {
                continue;
            }

            auto minViewX = size.x;
            auto maxViewX = -1;

            for (auto pictureX = pictureRect.a.x; pictureX < pictureRect.b.x; pictureX++) {
                auto viewX = pictureXToView(pictureX);
                if (viewX < 0 || viewX >= size.x) {
                    continue;
                }

                if (viewX < minViewX) {
                    minViewX = viewX;
                }
                if (viewX > maxViewX) {
                    maxViewX = viewX;
                }

                const auto& cell = picture.cells.at(pictureY * picture.width + pictureX);
                if (cell.transparent) {
                    if (flashingMask) {
                        b.moveStr(viewX - minViewX, "X", { 0x5F });
                    } else {
                        b.moveStr(viewX - minViewX, " ", { 0x5F });
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

    void drawSelectionBorder(int x, int y, bool dot) {
        auto ch = picture.cells.at(y * picture.width + x).ch;
        TDrawBuffer b;
        b.moveCStr(0, dot ? "•" : ch, { flashingSelection ? 0x2A : 0xA2 });
        writeBufferChar(x, y, b);
    }

    void draw() override {
        {
            TDrawBuffer b;
            b.moveChar(0, ' ', TColorAttr(0x87), size.x);
            for (auto y = 0; y < size.y; y++) {
                writeLine(0, static_cast<int16_t>(y), static_cast<int16_t>(size.x), 1, b);
            }
        }

        drawPictureRegion(TRect(0, 0, picture.width, picture.height));

        if (selection.has_value()) {
            auto left = selection->a.x;
            auto right = selection->b.x - 1;
            auto top = selection->a.y;
            auto bottom = selection->b.y - 1;
            for (auto y = selection->a.y; y < selection->b.y; y++) {
                drawSelectionBorder(left, y, false);
                drawSelectionBorder(right, y, false);
            }
            for (auto x = selection->a.x; x < selection->b.x; x++) {
                drawSelectionBorder(x, top, false);
                drawSelectionBorder(x, bottom, false);
            }
            if (flashingSelection) {
                drawSelectionBorder(left, top, true);
                drawSelectionBorder(left, bottom, true);
                drawSelectionBorder(right, top, true);
                drawSelectionBorder(right, bottom, true);
            } else if (right - left > 3 && bottom - top > 3) {
                drawSelectionBorder((left + right + 1) / 2, top, true);
                drawSelectionBorder((left + right + 1) / 2, bottom, true);
                drawSelectionBorder(left, (top + bottom + 1) / 2, true);
                drawSelectionBorder(right, (top + bottom + 1) / 2, true);
            }
        }

        // top and left grippers are disabled
        {
            TDrawBuffer b;
            b.moveChar(0, static_cast<char>(254), TColorAttr(0x80), 1);
            writeBufferChar(-2, -1, b);
            writeBufferChar(-2, picture.height / 2, b);
            writeBufferChar(-2, picture.height, b);
            writeBufferChar(picture.width / 2, -1, b);
            writeBufferChar(picture.width + 1, -1, b);
        }

        // bottom and right grippers are enabled
        {
            TDrawBuffer b;
            b.moveChar(0, static_cast<char>(254), TColorAttr(0x8F), 1);
            writeBufferChar(picture.width / 2, picture.height, b);
            writeBufferChar(picture.width + 1, picture.height / 2, b);
            writeBufferChar(picture.width + 1, picture.height, b);
        }
    }
};

enum class PictureWindowMode {
    kSelect,
    kDraw,
    kPick,
    kType,
    kMask,
};

class PictureWindowPrivate {
   public:
    int ticks = 0;

    TColorRGB fg{ 255, 255, 255 };
    TColorRGB bg{ 0, 0, 0 };
    std::string ch = "☺";
    PictureWindowMode mode = PictureWindowMode::kSelect;

    // select and type tool
    std::optional<TPoint> currentDrag;

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
    ViewPtr<Label> toolLabel{ TRect(1, 1, 15, 2) };
    ViewPtr<CheckBoxes> setFgCheck{ std::vector<std::string>{ "Set FG color" }, std::vector<bool>{ true } };
    ViewPtr<CheckBoxes> setBgCheck{ std::vector<std::string>{ "Set BG color" }, std::vector<bool>{ true } };
    ViewPtr<CheckBoxes> setChCheck{ std::vector<std::string>{ "Set character" }, std::vector<bool>{ true } };
    ViewPtr<Label> maskHelp{ "Click to toggle between opaque and transparent." };
    ViewPtr<ThinButton> cutButton{ "~Ctrl+X~ Cut", cmCut, 0 };
    ViewPtr<ThinButton> copyButton{ "~Ctrl+C~ Copy", cmCopy, 0 };
    ViewPtr<ThinButton> pasteButton{ "~Ctrl+V~ Paste", cmPaste, 0 };
    ViewPtr<ThinButton> clearButton{ "~Del~ Clear", cmPaste, 0 };
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

static void enableDisableSelectButtons(PictureWindowPrivate* p) {
    auto enabled = p->pictureView->selection.has_value();

    p->copyButton->setState(sfDisabled, !enabled);
    p->cutButton->setState(sfDisabled, !enabled);
    p->pasteButton->setState(sfDisabled, !enabled);
    p->clearButton->setState(sfDisabled, !enabled);

    p->copyButton->drawView();
    p->cutButton->drawView();
    p->pasteButton->drawView();
    p->clearButton->drawView();
}

static void onSelectionCut(PictureWindowPrivate* p) {}

static void onSelectionCopy(PictureWindowPrivate* p) {}

static void onSelectionPaste(PictureWindowPrivate* p) {}

static void onSelectionClear(PictureWindowPrivate* p) {
    if (!p->pictureView->selection.has_value()) {
        return;
    }

    PictureCell pictureCell;
    pictureCell.ch = p->ch;
    pictureCell.colorAttr = { p->fg, p->bg };
    pictureCell.transparent = false;

    const auto& selection = *p->pictureView->selection;
    auto& picture = p->pictureView->picture;
    for (auto y = selection.a.y; y < selection.b.y; y++) {
        for (auto x = selection.a.x; x < selection.b.x; x++) {
            picture.cells.at(y * picture.width + x) = pictureCell;
        }
    }

    p->pictureView->selection = {};
    enableDisableSelectButtons(p);
    p->pictureView->drawView();
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

    _private->setFgCheck->setBounds(TRect(15, 1, 33, 2));
    _private->setFgCheck.addTo(this);
    _private->setFgCheck->hide();

    _private->setBgCheck->setBounds(TRect(34, 1, 52, 2));
    _private->setBgCheck.addTo(this);
    _private->setBgCheck->hide();

    _private->setChCheck->setBounds(TRect(53, 1, 72, 2));
    _private->setChCheck.addTo(this);
    _private->setChCheck->hide();

    _private->maskHelp->setBounds(TRect(15, 1, 70, 2));
    _private->maskHelp.addTo(this);
    _private->maskHelp->hide();

    RowLayout(
        false,
        {
            _private->cutButton.take(),
            _private->copyButton.take(),
            _private->pasteButton.take(),
            _private->clearButton.take(),
        })
        .addTo(this, 15, 80, 1);

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

static void showHide(bool showCondition, std::initializer_list<TView*> views) {
    for (auto* view : views) {
        if (showCondition) {
            view->show();
        } else {
            view->hide();
        }
    }
}

static void updateStatusItems(PictureWindowPrivate* p) {
    std::ostringstream chText;
    chText << "~F3~ " << p->ch;
    delete[] p->statusItems.character->text;  // NOLINT
    p->statusItems.character->text = strdup(chText.str().c_str());

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
    p->statusItems.statusLine->drawView();

    std::string labelText;
    switch (p->mode) {
        case PictureWindowMode::kSelect:
            labelText = "Select tool";
            break;
        case PictureWindowMode::kDraw:
            labelText = "Draw tool";
            break;
        case PictureWindowMode::kPick:
            labelText = "Pick tool";
            break;
        case PictureWindowMode::kType:
            labelText = "Type tool";
            break;
        case PictureWindowMode::kMask:
            labelText = "Mask tool";
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
            enableDisableSelectButtons(p);
            p->pictureView->drawView();
        }
        if (p->mode == PictureWindowMode::kType && (sel.b.x > sel.a.x + 1 || sel.b.y > sel.a.y + 1)) {
            p->pictureView->selection = TRect(sel.a.x, sel.a.y, sel.a.x + 1, sel.a.y + 1);
            enableDisableSelectButtons(p);
            p->pictureView->drawView();
        }
    }

    // Mask help
    showHide(p->mode == PictureWindowMode::kMask, { p->maskHelp });

    p->toolLabel->setTitle(labelText);
    p->toolLabel->drawView();
}

static void onMouse(int pictureX, int pictureY, const PictureViewMouseEventArgs& e, PictureWindowPrivate* p) {
    TPoint pt{ pictureX, pictureY };
    auto& picture = p->pictureView->picture;
    auto leftMouseDown = e.down && (e.buttons & mbLeftButton) != 0;
    auto dragging = e.move && (e.buttons & mbLeftButton) != 0;

    TPoint verticalGripper{ picture.width / 2, picture.height };
    TPoint horizontalGripper{ picture.width + 1, picture.height / 2 };
    TPoint diagonalGripper{ picture.width + 1, picture.height };
    if (leftMouseDown && (pt == verticalGripper || pt == horizontalGripper || pt == diagonalGripper)) {
        p->resizing = true;
        p->resizingStartPicturePoint = pt;
        p->resizingOriginalPicture = picture;
        p->resizingVertical = pt == verticalGripper || pt == diagonalGripper;
        p->resizingHorizontal = pt == horizontalGripper || pt == diagonalGripper;
        p->pictureView->selection = {};
        enableDisableSelectButtons(p);
        p->pictureView->drawView();
        return;
    }
    if (dragging && p->resizing) {
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
        enableDisableSelectButtons(p);
        p->pictureView->drawView();
        return;
    }
    if (e.up && p->resizing) {
        p->resizing = false;
        p->resizingOriginalPicture = {};
        updateScrollBars(p);
        return;
    }

    if (pt.x < 0 || pt.x >= picture.width || pt.y < 0 || pt.y >= picture.height) {
        return;
    }

    auto& cell = picture.cells.at(pictureY * picture.width + pictureX);
    TColorAttr color{ p->fg, p->bg };

    switch (p->mode) {
        case PictureWindowMode::kSelect:
            if (p->currentDrag.has_value()) {
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
                enableDisableSelectButtons(p);
            }
            if (e.down) {
                p->currentDrag = TPoint{ pictureX, pictureY };
                p->pictureView->selection =
                    TRect(*p->currentDrag, TPoint{ p->currentDrag->x + 1, p->currentDrag->y + 1 });
                enableDisableSelectButtons(p);
            } else if ((e.up || (e.move && e.buttons == 0)) && p->currentDrag.has_value()) {
                p->currentDrag = {};
            }
            p->pictureView->drawView();
            break;

        case PictureWindowMode::kDraw:
            if (leftMouseDown || dragging) {
                if (p->setFgCheck->mark(0)) {
                    cell.colorAttr._fg = color._fg;
                    cell.colorAttr._style = color._style;
                }
                if (p->setBgCheck->mark(0)) {
                    cell.colorAttr._bg = color._bg;
                }
                if (p->setChCheck->mark(0)) {
                    cell.ch = p->ch;
                }
                cell.transparent = false;
                p->pictureView->drawView();
            }
            break;

        case PictureWindowMode::kPick:
            if (leftMouseDown || dragging) {
                if (p->setBgCheck->mark(0)) {
                    p->bg = getBack(cell.colorAttr).asRGB();
                }
                if (p->setFgCheck->mark(0)) {
                    p->fg = getFore(cell.colorAttr).asRGB();
                }
                if (p->setChCheck->mark(0)) {
                    p->ch = cell.ch;
                }
                updateStatusItems(p);
            }
            break;

        case PictureWindowMode::kType:
            if (leftMouseDown || dragging) {
                p->pictureView->selection = TRect(pictureX, pictureY, pictureX + 1, pictureY + 1);
                enableDisableSelectButtons(p);
                p->pictureView->drawView();
            }
            break;

        case PictureWindowMode::kMask:
            if (leftMouseDown) {
                p->currentDragTransparent = !cell.transparent;
            }
            if (leftMouseDown || dragging) {
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
        } else if (p->mode == PictureWindowMode::kSelect || p->mode == PictureWindowMode::kType) {
            p->pictureView->flashingSelection = !p->pictureView->flashingSelection;
            p->pictureView->drawView();
        }
        p->ticks = 0;
    }
}

void PictureWindow::handleEvent(TEvent& event) {
    if (event.what == evKeyboard) {
        if (event.keyDown.keyCode == kbEsc && _private->mode == PictureWindowMode::kSelect) {
            _private->pictureView->selection = {};
            enableDisableSelectButtons(_private);
            _private->pictureView->drawView();
            clearEvent(event);
        } else if (
            _private->mode == PictureWindowMode::kType && _private->pictureView->selection.has_value() &&
            event.keyDown.text[0] != '\0' && event.keyDown.keyCode != kbLeft && event.keyDown.keyCode != kbRight &&
            event.keyDown.keyCode != kbUp && event.keyDown.keyCode != kbDown) {
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
            enableDisableSelectButtons(_private);
            _private->pictureView->drawView();
            clearEvent(event);
        } else if (
            (_private->mode == PictureWindowMode::kSelect || _private->mode == PictureWindowMode::kType) &&
            (event.keyDown.keyCode == kbLeft || event.keyDown.keyCode == kbRight || event.keyDown.keyCode == kbUp ||
             event.keyDown.keyCode == kbDown) &&
            _private->pictureView->selection.has_value()) {
            auto deltaX = 0;
            auto deltaY = 0;
            switch (event.keyDown.keyCode) {
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
            auto r = *_private->pictureView->selection;
            r.a.x += deltaX;
            r.b.x += deltaX;
            r.a.y += deltaY;
            r.b.y += deltaY;
            auto width = _private->pictureView->picture.width;
            auto height = _private->pictureView->picture.height;
            if (r.a.x >= 0 && r.a.x <= width && r.b.x >= 0 && r.b.x <= width && r.a.y >= 0 && r.a.y <= height &&
                r.b.y >= 0 && r.b.y <= height) {
                _private->pictureView->selection = r;
                enableDisableSelectButtons(_private);
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

            case cmClear:
                if (_private->mode == PictureWindowMode::kSelect) {
                    onSelectionClear(_private);
                }
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
                if (_private->mode == PictureWindowMode::kSelect) {
                    onSelectionPaste(_private);
                }
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
        (enable ? enableCommands : disableCommands)(ts);
    }
}

void PictureWindow::changeBounds(const TRect& bounds) {
    TWindow::changeBounds(bounds);
    updateScrollBars(_private);
}

void PictureWindow::onStatusLineCommand(ushort cmd) {
    if (cmd != kCmdPictureMask) {
        _private->pictureView->flashingMask = false;
        _private->pictureView->drawView();
    }

    switch (cmd) {
        case kCmdPictureFg: {
            auto dialog = DialogPtr<InsertColorDialog>("Choose Foreground Color", "Choose");
            if (TProgram::deskTop->execView(dialog) == cmOK) {
                _private->fg = dialog->selection;
                updateStatusItems(_private);
            }
            break;
        }

        case kCmdPictureBg: {
            auto dialog = DialogPtr<InsertColorDialog>("Choose Background Color", "Choose");
            if (TProgram::deskTop->execView(dialog) == cmOK) {
                _private->bg = dialog->selection;
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
                enableDisableSelectButtons(_private);
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
                title = strdup(getPictureWindowTitle(_private->pictureView->picture.name).c_str());
                frame->drawView();
                _private->onEdited();
            }
            break;
        }

        default:
            break;
    }
}

}  // namespace tmbasic
