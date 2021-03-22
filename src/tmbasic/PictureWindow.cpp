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
using util::Button;
using util::CheckBoxes;
using util::DialogPtr;
using util::Label;
using util::ScrollBar;
using util::ViewPtr;
using vm::UserForm;

namespace tmbasic {

class PictureCell {
   public:
    bool transparent = false;
    TColorAttr colorAttr{ TColorDesired(TColorBIOS(0)), TColorDesired(TColorBIOS(0)) };
    std::string ch = " ";
};

class Picture {
   public:
    std::vector<PictureCell> cells;
    int width;
    int height;

    Picture(int width, int height) : cells(width * height, PictureCell()), width(width), height(height) {
        for (auto i = 0; i < height; i++) {
            for (auto j = 0; j < width; j++) {
                cells.at(i * width + j).ch = '0' + (i * 17 + j) % 78;
                cells.at(i * width + j).colorAttr = TColorAttr(
                    TColorDesired(TColorBIOS(15)), TColorDesired(TColorRGB(i * 255 / height, j * 255 / width, 255)));
            }
        }
    }
};

class PictureView : public TView {
   public:
    Picture picture{ 40, 15 };
    int scrollTop = 0;
    int scrollLeft = 0;

    explicit PictureView(const TRect& bounds) : TView(bounds) {}

    int pictureXToView(int x) { return x - scrollLeft + 2; }

    int pictureYToView(int y) { return y - scrollTop + 1; }

    void writeBufferChar(int pictureX, int pictureY, const TDrawBuffer& b) {
        auto viewX = pictureXToView(pictureX);
        auto viewY = pictureYToView(pictureY);
        if (viewX >= 0 && viewX < size.x && viewY >= 0 && viewY < size.y) {
            writeLine(viewX, viewY, 1, 1, b);
        }
    }

    void draw() override {
        {
            TDrawBuffer b;
            b.moveChar(0, ' ', TColorAttr(TColorDesired(TColorBIOS(7)), TColorDesired(TColorBIOS(8))), size.x);
            for (auto y = 0; y < size.y; y++) {
                writeLine(0, y, size.x, 1, b);
            }
        }

        for (auto pictureY = 0; pictureY < picture.height; pictureY++) {
            TDrawBuffer b;
            auto viewY = pictureYToView(pictureY);
            if (viewY < 0 || viewY >= size.y) {
                continue;
            }

            auto minViewX = size.x;
            auto maxViewX = -1;

            for (auto pictureX = 0; pictureX < picture.width; pictureX++) {
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
    TColorRGB fg{ 255, 255, 255 };
    TColorRGB bg{ 0, 0, 0 };
    const char* ch = "\x03";
    PictureWindowMode mode = PictureWindowMode::kSelect;

    compiler::SourceMember* member;
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
}

PictureWindow::~PictureWindow() {
    delete _private;
}

static void updateStatusItems(PictureWindowPrivate* p) {
    std::ostringstream chText;
    chText << "~F3~:[" << p->ch << "]";
    delete[] p->statusItems.character->text;
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

    p->toolLabel->setTitle(labelText);
    p->toolLabel->drawView();
}

void PictureWindow::handleEvent(TEvent& event) {
    TWindow::handleEvent(event);

    if (event.what == evBroadcast) {
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

            case cmScrollBarChanged: {
                _private->pictureView->scrollTop = _private->vScrollBar->value;
                _private->pictureView->scrollLeft = _private->hScrollBar->value;
                _private->pictureView->drawView();
                break;
            }
        }
    }
}

uint16_t PictureWindow::getHelpCtx() {
    return hcide_pictureWindow;
}

void PictureWindow::close() {
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
