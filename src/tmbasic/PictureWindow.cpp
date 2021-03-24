#include "tmbasic/PictureWindow.h"
#include "../../obj/resources/help/helpfile.h"
#include "../util/Button.h"
#include "../util/CheckBoxes.h"
#include "../util/DialogPtr.h"
#include "../util/Label.h"
#include "../util/ScrollBar.h"
#include "../util/ViewPtr.h"
#include "../vm/UserForm.h"
#include "App.h"
#include "RowLayout.h"
#include "events.h"
#include "tmbasic/InsertColorDialog.h"
#include "tmbasic/InsertSymbolDialog.h"

using compiler::SourceMember;
using util::CheckBoxes;
using util::DialogPtr;
using util::Label;
using util::ScrollBar;
using util::ViewPtr;

namespace tmbasic {

class PictureCell {
   public:
    bool transparent = false;
    TColorAttr colorAttr{ 0 };
    std::string ch = " ";
};

static uint32_t packRgb(TColorRGB rgb) {
    uint32_t x = rgb.r;
    x <<= 8;
    x |= rgb.g;
    x <<= 8;
    x |= rgb.b;
    return x;
}

static TColorRGB unpackRgb(uint32_t packed) {
    auto b = static_cast<uint8_t>(packed & 0xFF);
    packed >>= 8;
    auto g = static_cast<uint8_t>(packed & 0xFF);
    packed >>= 8;
    auto r = static_cast<uint8_t>(packed & 0xFF);
    return { r, g, b };
}

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

class Picture {
   public:
    std::string name = "Untitled";
    std::vector<PictureCell> cells;
    int width;
    int height;

    Picture(int width, int height) : cells(width * height, PictureCell()), width(width), height(height) {
        for (auto i = 0; i < height; i++) {
            for (auto j = 0; j < width; j++) {
                cells.at(i * width + j).ch = '0' + (i * 17 + j) % 78;
                cells.at(i * width + j).colorAttr = { TColorRGB(255, 255, 255),
                                                      TColorRGB(i * 255 / height, j * 255 / width, 255) };
            }
        }
    }

    Picture(const std::string& source) {
        std::istringstream s{ source };
        s >> std::hex;
        std::string pictureKeyword = "", sizeSeparator = "";
        s >> pictureKeyword >> name >> width >> height >> sizeSeparator;
        if (pictureKeyword != "picture" || sizeSeparator != ":") {
            throw std::runtime_error("Unexpected data in picture source.");
        }
        cells = { static_cast<size_t>(width * height), PictureCell() };
        for (auto y = 0; y < height; y++) {
            for (auto x = 0; x < width; x++) {
                std::string utf8Hex;
                int transparent = 0;
                uint32_t fg = 0, bg = 0;
                std::string cellSeparator = "";
                s >> utf8Hex >> transparent >> fg >> bg >> cellSeparator;
                if (cellSeparator != ";") {
                    throw std::runtime_error("Unexpected data in picture source.");
                }
                auto& cell = cells.at(y * width + x);
                cell.transparent = transparent != 0;
                cell.colorAttr = { unpackRgb(fg), unpackRgb(bg) };
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
        s << "picture " << name << "\n" << std::hex << width << " " << height << " : ";
        for (auto y = 0; y < height; y++) {
            for (auto x = 0; x < width; x++) {
                auto n = y * width + x;
                if ((n % 6) == 0) {
                    s << "\n";
                }
                auto& cell = cells.at(n);
                for (auto ch : cell.ch) {
                    s << std::setw(2) << std::setfill('0') << static_cast<int>(ch);
                }
                s << " ";

                s << (cell.transparent ? 1 : 0) << " " << packRgb(getFore(cell.colorAttr).asRGB()) << " "
                  << packRgb(getBack(cell.colorAttr).asRGB()) << " ; ";
            }
        }
        s << "\nend picture\n";
        return s.str();
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
    std::optional<TRect> selection;
    bool flashingSelection = true;

    explicit PictureView(const TRect& bounds) : TView(bounds) {
        eventMask = evMouseDown | evMouseMove | evMouseUp | evCommand | evBroadcast;
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
                b.moveStr(viewX - minViewX, cell.ch, cell.colorAttr);
            }

            if (maxViewX >= minViewX) {
                writeLine(minViewX, viewY, maxViewX - minViewX + 1, 1, b);
            }
        }
    }

    void drawSelectionDot(int x, int y) {
        auto ch = picture.cells.at(y * picture.width + x).ch;
        TDrawBuffer b;
        b.moveCStr(
            0, ch, TColorAttr(TColorDesired(TColorBIOS(7)), TColorDesired(TColorBIOS(flashingSelection ? 0 : 15))));
        writeBufferChar(x, y, b);
    }

    void draw() override {
        {
            TDrawBuffer b;
            b.moveChar(0, ' ', TColorAttr(TColorDesired(TColorBIOS(7)), TColorDesired(TColorBIOS(8))), size.x);
            for (auto y = 0; y < size.y; y++) {
                writeLine(0, y, size.x, 1, b);
            }
        }

        drawPictureRegion(TRect(0, 0, picture.width, picture.height));

        if (selection.has_value()) {
            for (auto y = selection->a.y; y < selection->b.y; y++) {
                drawSelectionDot(selection->a.x, y);
                drawSelectionDot(selection->b.x - 1, y);
            }
            for (auto x = selection->a.x; x < selection->b.x; x++) {
                drawSelectionDot(x, selection->a.y);
                drawSelectionDot(x, selection->b.y - 1);
            }
        }

        // top and left grippers are disabled
        {
            TDrawBuffer b;
            b.moveChar(0, 254, TColorAttr(TColorDesired(TColorBIOS(0)), TColorDesired(TColorBIOS(8))), 1);
            writeBufferChar(-2, -1, b);
            writeBufferChar(-2, picture.height / 2, b);
            writeBufferChar(-2, picture.height, b);
            writeBufferChar(picture.width / 2, -1, b);
            writeBufferChar(picture.width + 1, -1, b);
        }

        // bottom and right grippers are enabled
        {
            TDrawBuffer b;
            b.moveChar(0, 254, TColorAttr(TColorDesired(TColorBIOS(15)), TColorDesired(TColorBIOS(8))), 1);
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
    kText,
    kMask,
};

class PictureWindowPrivate {
   public:
    int ticks = 0;

    TColorRGB fg{ 255, 255, 255 };
    TColorRGB bg{ 0, 0, 0 };
    const char* ch = "☺";
    PictureWindowMode mode = PictureWindowMode::kSelect;
    std::optional<TPoint> currentDrag;

    compiler::SourceMember* member{};
    std::function<void()> onEdited;
    PictureWindowStatusItems statusItems;

    ViewPtr<PictureView> pictureView{ TRect() };
    ViewPtr<ScrollBar> vScrollBar{ TRect(0, 0, 1, 10) };
    ViewPtr<ScrollBar> hScrollBar{ TRect(0, 0, 10, 1) };
    ViewPtr<Label> toolLabel{ TRect(1, 1, 15, 2) };
    ViewPtr<CheckBoxes> setFgCheck{ std::vector<std::string>{ "Set FG" }, std::vector<bool>{ true } };
    ViewPtr<CheckBoxes> setBgCheck{ std::vector<std::string>{ "Set BG" }, std::vector<bool>{ true } };
    ViewPtr<CheckBoxes> setChCheck{ std::vector<std::string>{ "Set Character" }, std::vector<bool>{ true } };
};

static std::string getPictureWindowTitle(const SourceMember& member) {
    return member.identifier + " (Picture)";
}

static void updateScrollBars(PictureWindowPrivate* p) {
    p->vScrollBar->setParams(
        0, 0, std::max(0, p->pictureView->picture.height + 5 - (p->pictureView->size.y - 1)),
        p->pictureView->size.y - 1, 1);
    p->hScrollBar->setParams(
        0, 0, std::max(0, p->pictureView->picture.width + 10 - (p->pictureView->size.x - 1)),
        p->pictureView->size.x - 1, 1);
}

PictureWindow::PictureWindow(
    const TRect& r,
    SourceMember* member,
    std::function<void()> onEdited,
    const PictureWindowStatusItems& statusItems)
    : TWindow(r, getPictureWindowTitle(*member), wnNoNumber),
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

    _private->pictureView->setBounds(TRect(1, 2, size.x - 1, size.y - 1));
    _private->pictureView->growMode = gfGrowHiX | gfGrowHiY;
    insert(_private->pictureView.take());

    _private->toolLabel.addTo(this);
    _private->setFgCheck->setBounds(TRect(14, 1, 26, 2));
    insert(_private->setFgCheck.take());
    _private->setFgCheck->hide();
    _private->setBgCheck->setBounds(TRect(27, 1, 39, 2));
    insert(_private->setBgCheck.take());
    _private->setBgCheck->hide();
    _private->setChCheck->setBounds(TRect(40, 1, 59, 2));
    insert(_private->setChCheck.take());
    _private->setChCheck->hide();

    updateScrollBars(_private);

    try {
        _private->pictureView->picture = Picture(member->source);
    } catch (...) {
    }
}

PictureWindow::~PictureWindow() {
    delete _private;
}

static void updateStatusItems(PictureWindowPrivate* p) {
    std::ostringstream chText;
    chText << "~F3~:[" << p->ch << "]";
    delete[] p->statusItems.character->text;  // NOLINT
    p->statusItems.character->text = strdup(chText.str().c_str());

    auto attrPair = TAttrPair(
        TColorAttr(TColorDesired(p->fg), TColorDesired(p->bg)), TColorAttr(TColorDesired(p->fg), TColorDesired(p->bg)));
    p->statusItems.fgColor->colorPairNormal = attrPair;
    p->statusItems.bgColor->colorPairNormal = attrPair;
    p->statusItems.characterColor->colorPairNormal = attrPair;

    auto sel = TAttrPair(
        TColorAttr(TColorDesired(TColorBIOS(15)), TColorDesired(TColorBIOS(2))),
        TColorAttr(TColorDesired(TColorBIOS(15)), TColorDesired(TColorBIOS(2))));
    auto unsel = TAttrPair(
        TColorAttr(TColorDesired(TColorBIOS(0)), TColorDesired(TColorBIOS(7))),
        TColorAttr(TColorDesired(TColorBIOS(4)), TColorDesired(TColorBIOS(7))));

    p->statusItems.selectColor->colorPairNormal = p->mode == PictureWindowMode::kSelect ? sel : unsel;
    p->statusItems.drawColor->colorPairNormal = p->mode == PictureWindowMode::kDraw ? sel : unsel;
    p->statusItems.pickColor->colorPairNormal = p->mode == PictureWindowMode::kPick ? sel : unsel;
    p->statusItems.textColor->colorPairNormal = p->mode == PictureWindowMode::kText ? sel : unsel;
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
        case PictureWindowMode::kText:
            labelText = "Text tool";
            break;
        case PictureWindowMode::kMask:
            labelText = "Mask tool";
            break;
        default:
            assert(false);
            break;
    }

    // Set FG, Set BG
    if (p->mode == PictureWindowMode::kDraw || p->mode == PictureWindowMode::kText ||
        p->mode == PictureWindowMode::kPick) {
        p->setBgCheck->show();
        p->setFgCheck->show();
    } else {
        p->setBgCheck->hide();
        p->setFgCheck->hide();
    }

    // Set Char
    if (p->mode == PictureWindowMode::kDraw || p->mode == PictureWindowMode::kPick) {
        p->setChCheck->show();
    } else {
        p->setChCheck->hide();
    }

    // Selection
    if (p->pictureView->selection.has_value()) {
        auto sel = *p->pictureView->selection;
        if (p->mode != PictureWindowMode::kSelect && p->mode != PictureWindowMode::kText) {
            p->pictureView->selection = {};
            p->pictureView->drawView();
        }
        if (p->mode == PictureWindowMode::kText && (sel.b.x > sel.a.x + 1 || sel.b.y > sel.a.y + 1)) {
            p->pictureView->selection = TRect(sel.a.x, sel.a.y, sel.a.x + 1, sel.a.y + 1);
            p->pictureView->drawView();
        }
    }

    p->toolLabel->setTitle(labelText);
    p->toolLabel->drawView();
}

static void onMouse(int pictureX, int pictureY, const PictureViewMouseEventArgs& e, PictureWindowPrivate* p) {
    auto& cell = p->pictureView->picture.cells.at(pictureY * p->pictureView->picture.width + pictureX);
    auto color = TColorAttr(TColorDesired(p->fg), TColorDesired(p->bg));
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
            }
            if (e.down) {
                p->currentDrag = TPoint{ pictureX, pictureY };
                p->pictureView->selection =
                    TRect(*p->currentDrag, TPoint{ p->currentDrag->x + 1, p->currentDrag->y + 1 });
            } else if ((e.up || (e.move && e.buttons == 0)) && p->currentDrag.has_value()) {
                p->currentDrag = {};
            }
            p->pictureView->drawView();
            break;

        case PictureWindowMode::kDraw:
            if (e.down || e.move) {
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
                p->pictureView->drawView();
            }
            break;

        case PictureWindowMode::kPick:
            if (e.down || e.move) {
                if (p->setBgCheck->mark(0)) {
                    p->bg = getBack(cell.colorAttr).asRGB();
                }
                if (p->setFgCheck->mark(0)) {
                    p->fg = getFore(cell.colorAttr).asRGB();
                }
                if (p->setChCheck->mark(0)) {
                    p->ch = cell.ch.c_str();
                }
                updateStatusItems(p);
            }
            break;

        case PictureWindowMode::kText:
            p->pictureView->selection = TRect(pictureX, pictureY, pictureX + 1, pictureY + 1);
            p->pictureView->drawView();
            break;

        default:
            break;
    }
}

static void onTick(PictureWindowPrivate* p) {
    // blink the selection rectangle
    p->ticks++;
    if (p->ticks >= 2) {
        p->pictureView->flashingSelection = !p->pictureView->flashingSelection;
        if (p->pictureView->selection.has_value()) {
            p->pictureView->drawView();
        }
        p->ticks = 0;
    }
}

void PictureWindow::handleEvent(TEvent& event) {
    TWindow::handleEvent(event);

    if (event.what == evKeyDown) {
        if (event.keyDown.keyCode == kbEsc && _private->mode == PictureWindowMode::kSelect) {
            _private->pictureView->selection = {};
            _private->pictureView->drawView();
        } else if (
            _private->mode == PictureWindowMode::kText && _private->pictureView->selection.has_value() &&
            event.keyDown.text[0] != '\0') {
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

            if (rect.a.x < _private->pictureView->picture.width - 1) {
                rect.a.x++;
                rect.b.x++;
            }
            _private->pictureView->selection = rect;
            _private->pictureView->drawView();
        }
    } else if (event.what == evCommand) {
        switch (event.message.command) {
            case kCmdPictureViewMouse: {
                auto* e = reinterpret_cast<PictureViewMouseEventArgs*>(event.message.infoPtr);  // NOLINT
                auto pictureX = _private->pictureView->viewXToPicture(e->viewPoint.x);
                auto pictureY = _private->pictureView->viewYToPicture(e->viewPoint.y);
                if (pictureX >= 0 && pictureX < _private->pictureView->picture.width && pictureY >= 0 &&
                    pictureY < _private->pictureView->picture.height) {
                    onMouse(pictureX, pictureY, *e, _private);
                }
                break;
            }
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
        ts.enableCmd(kCmdPictureText);
        ts.enableCmd(kCmdPictureMask);
        (enable ? enableCommands : disableCommands)(ts);
    }
}

void PictureWindow::changeBounds(const TRect& bounds) {
    TWindow::changeBounds(bounds);
    updateScrollBars(_private);
}

void PictureWindow::onStatusLineCommand(ushort cmd) {
    switch (cmd) {
        case kCmdPictureFg: {
            auto dialog = DialogPtr<InsertColorDialog>("Choose Foreground Color", "Choose");
            if (TProgram::deskTop->execView(dialog) == cmOK) {
                _private->fg = dialog->selection.asRGB();
                updateStatusItems(_private);
            }
            break;
        }

        case kCmdPictureBg: {
            auto dialog = DialogPtr<InsertColorDialog>("Choose Background Color", "Choose");
            if (TProgram::deskTop->execView(dialog) == cmOK) {
                _private->bg = dialog->selection.asRGB();
                updateStatusItems(_private);
            }
            break;
        }

        case kCmdPictureCharacter: {
            auto dialog = DialogPtr<InsertSymbolDialog>("Choose Character", "Choose");
            if (TProgram::deskTop->execView(dialog) == cmOK) {
                _private->ch = dialog->selection;
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
        case kCmdPictureText:
            _private->mode = PictureWindowMode::kText;
            updateStatusItems(_private);
            break;
        case kCmdPictureMask:
            _private->mode = PictureWindowMode::kMask;
            updateStatusItems(_private);
            break;

        default:
            break;
    }
}

}  // namespace tmbasic
