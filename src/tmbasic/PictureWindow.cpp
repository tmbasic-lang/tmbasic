#include "tmbasic/PictureWindow.h"
#include "../../obj/resources/help/helpfile.h"
#include "../util/Button.h"
#include "../util/DialogPtr.h"
#include "../util/Label.h"
#include "../util/ScrollBar.h"
#include "../util/ViewPtr.h"
#include "../vm/UserForm.h"
#include "RowLayout.h"
#include "events.h"

using compiler::SourceMember;
using util::Button;
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
    char ch = ' ';
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
    void draw() override {
        {
            TDrawBuffer b;
            b.moveChar(0, ' ', TColorAttr(TColorDesired(TColorBIOS(7)), TColorDesired(TColorBIOS(8))), size.x);
            for (auto y = 0; y < size.y; y++) {
                writeLine(0, y, size.x, 1, b);
            }
        }

        auto displayLines = std::min(picture.height - scrollTop, size.y);
        auto displayColumns = std::min(picture.width - scrollLeft, size.x);

        for (int i = 0; i < displayLines; i++) {
            TDrawBuffer b;
            for (int j = 0; j < displayColumns; j++) {
                const auto& cell = picture.cells.at((scrollTop + i) * picture.width + scrollLeft + j);
                b.moveChar(j, cell.ch, cell.colorAttr, 1);
            }
            writeLine(0, i, displayColumns, 1, b);
        }

        {
            auto centerX = (picture.width / 2) - scrollLeft;
            auto centerY = (picture.height / 2) - scrollTop;
            TDrawBuffer b;
            b.moveChar(
                0, static_cast<char>(254), TColorAttr(TColorDesired(TColorBIOS(7)), TColorDesired(TColorBIOS(8))), 1);
            writeLine(centerX, displayLines, 1, 1, b);
            writeLine(displayColumns, displayLines, 1, 1, b);
            writeLine(displayColumns, centerY, 1, 1, b);
        }
    }
};

class PictureWindowPrivate {
   public:
    compiler::SourceMember* member;
    std::function<void()> onEdited;
    ViewPtr<Button> selectToolButton{ TRect(1, 1, 13, 1 + 2), "~S~elect ", cmOK, 0 };
    ViewPtr<Button> drawToolButton{ TRect(1, 3, 13, 3 + 2), "~D~raw   ", cmOK, 0 };
    ViewPtr<Button> colorToolButton{ TRect(1, 5, 13, 5 + 2), "~C~olor  ", cmOK, 0 };
    ViewPtr<Button> textToolButton{ TRect(1, 7, 13, 7 + 2), "~T~ext   ", cmOK, 0 };
    ViewPtr<Button> shapeToolButton{ TRect(1, 9, 13, 9 + 2), "Sha~p~e  ", cmOK, 0 };
    ViewPtr<Button> charToolButton{ TRect(1, 12, 13, 12 + 2), "C~h~: _  ", cmOK, 0 };
    ViewPtr<Button> fgToolButton{ TRect(1, 14, 7, 14 + 2), "~F~G", cmOK, 0 };
    ViewPtr<Button> bgToolButton{ TRect(7, 14, 13, 14 + 2), "~B~G", cmOK, 0 };
    ViewPtr<Button> optionsButton{ TRect(1, 17, 13, 17 + 2), "~O~ptions", cmOK, 0 };
    ViewPtr<PictureView> pictureView{ TRect() };
    ViewPtr<ScrollBar> vScrollBar{ TRect(0, 0, 1, 10) };
    ViewPtr<ScrollBar> hScrollBar{ TRect(0, 0, 10, 1) };
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

PictureWindow::PictureWindow(const TRect& r, SourceMember* member, std::function<void()> onEdited)
    : TWindow(r, getPictureWindowTitle(*member), wnNoNumber),
      TWindowInit(TWindow::initFrame),
      _private(new PictureWindowPrivate()) {
    options |= ofTileable;

    _private->member = member;
    _private->onEdited = std::move(onEdited);

    _private->vScrollBar->setBounds(TRect(size.x - 1, 1, size.x, size.y - 1));
    _private->vScrollBar->useWhiteColorScheme();
    _private->vScrollBar.addTo(this);

    _private->hScrollBar->setBounds(TRect(14, size.y - 1, size.x - 2, size.y));
    _private->hScrollBar->useWhiteColorScheme();
    _private->hScrollBar.addTo(this);

    _private->pictureView->setBounds(TRect(14, 1, size.x - 1, size.y - 1));
    _private->pictureView->growMode = gfGrowHiX | gfGrowHiY;
    insert(_private->pictureView.take());

    updateScrollBars(_private);

    _private->selectToolButton->useWhiteColorScheme();
    insert(_private->selectToolButton.take());
    insert(_private->drawToolButton.take());
    insert(_private->colorToolButton.take());
    insert(_private->textToolButton.take());
    insert(_private->shapeToolButton.take());
    insert(_private->charToolButton.take());
    insert(_private->fgToolButton.take());
    insert(_private->bgToolButton.take());
    insert(_private->optionsButton.take());
}

PictureWindow::~PictureWindow() {
    delete _private;
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

    // if (aState == sfActive) {
    //     TCommandSet ts;
    //     ts.enableCmd(kCmdDesignAddButton);
    //     ts.enableCmd(kCmdDesignAddCheckBox);
    //     ts.enableCmd(kCmdDesignAddGroupBox);
    //     ts.enableCmd(kCmdDesignAddLabel);
    //     ts.enableCmd(kCmdDesignAddListBox);
    //     ts.enableCmd(kCmdDesignAddRadioButton);
    //     ts.enableCmd(kCmdDesignAddScrollBar);
    //     ts.enableCmd(kCmdDesignAddTextBox);
    //     ts.enableCmd(kCmdDesignAddCustomControl);
    //     (enable ? enableCommands : disableCommands)(ts);
    // }
}

void PictureWindow::changeBounds(const TRect& bounds) {
    TWindow::changeBounds(bounds);
    updateScrollBars(_private);
}

}  // namespace tmbasic
