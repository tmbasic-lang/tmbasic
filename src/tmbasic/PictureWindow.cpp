#include "PictureWindow.h"
#include "../../obj/resources/help/helpfile.h"
#include "../util/Button.h"
#include "../util/CheckBoxes.h"
#include "../util/DialogPtr.h"
#include "../util/InputLine.h"
#include "../util/Label.h"
#include "../util/PictureView.h"
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
#include "UndoStack.h"
#include "events.h"

using compiler::SourceMember;
using util::Button;
using util::CheckBoxes;
using util::DialogPtr;
using util::InputLine;
using util::Label;
using util::Picture;
using util::PictureCell;
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

class CanvasViewMouseEventArgs {
   public:
    TPoint viewPoint;
    uint8_t buttons;
    bool down;
    bool move;
    bool up;
};

class CanvasView : public TView {
   public:
    Picture picture{ 40, 15 };
    int scrollTop = 0;
    int scrollLeft = 0;

    // select and type modes
    std::optional<TRect> selection;
    bool flashingSelection = false;

    // select mode > paste and move operations
    std::optional<Picture> pastedPicture{};
    std::optional<TPoint> pastedPictureLocation{};

    // select mode > move operation
    std::optional<TRect> moveOriginalRect{};        // where the pastedPicture is being dragged from
    std::optional<TColorAttr> moveOriginalColor{};  // the color to fill in the original area

    // mask mode
    bool flashingMask = false;

    explicit CanvasView(const TRect& bounds) : TView(bounds) {
        eventMask = evMouseDown | evMouseMove | evMouseUp;
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
            CanvasViewMouseEventArgs e{ lastMouse, event.mouse.buttons, event.what == evMouseDown,
                                        event.what == evMouseMove, event.what == evMouseUp };
            message(owner, evCommand, kCmdPictureViewMouse, &e);
            clearEvent(event);
        }
    }

    void drawPictureRegion(const Picture& p, int xViewOffset = 0, int yViewOffset = 0, bool isMainDocPhoto = true) {
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
                if (isMainDocPhoto && moveOriginalRect.has_value() &&
                    moveOriginalRect->contains(TPoint{ pictureX, pictureY })) {
                    b.moveChar(viewX - minViewX, ' ', *moveOriginalColor, 1);
                } else if (cell.transparent) {
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
        // the pasted picture may be either a literal pasted picture, or the user dragging a selection to move it.
        if (pastedPicture.has_value()) {
            auto x = 0;
            auto y = 0;
            if (pastedPictureLocation.has_value()) {
                x = pastedPictureLocation->x;
                y = pastedPictureLocation->y;
            }
            drawPictureRegion(*pastedPicture, x, y, false);
            drawSelectionRect(
                *pastedPicture, { x, y, x + pastedPicture->width, y + pastedPicture->height }, x, y,
                moveOriginalRect.has_value() ? 0x2A : 0x3B);
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

    void commitPaste() {
        const auto& src = *pastedPicture;
        auto& dst = picture;

        if (moveOriginalRect.has_value()) {
            for (auto yDst = moveOriginalRect->a.y; yDst < moveOriginalRect->b.y; yDst++) {
                for (auto xDst = moveOriginalRect->a.x; xDst < moveOriginalRect->b.x; xDst++) {
                    auto& cellDst = dst.cells.at(yDst * dst.width + xDst);
                    cellDst = PictureCell{ false, *moveOriginalColor, " " };
                }
            }
        }

        for (auto ySrc = 0; ySrc < src.height; ySrc++) {
            auto yDst = ySrc + pastedPictureLocation->y;
            if (yDst < 0 || yDst >= dst.height) {
                continue;
            }
            for (auto xSrc = 0; xSrc < src.width; xSrc++) {
                auto xDst = xSrc + pastedPictureLocation->x;
                if (xDst < 0 || xDst >= dst.width) {
                    continue;
                }
                const auto& cellSrc = src.cells.at(ySrc * src.width + xSrc);
                auto& cellDst = dst.cells.at(yDst * dst.width + xDst);
                cellDst = cellSrc;
            }
        }
    }
};

class PictureOptionsDialog : public TDialog {
   public:
    int selectedWidth = -1;
    int selectedHeight = -1;
    std::string selectedName = "";

    explicit PictureOptionsDialog(Picture* picture)
        : TDialog(TRect(0, 0, 0, 0), "Picture Options"),
          TWindowInit(&TDialog::initFrame),
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
                util::validateIdentifier(_nameText->data, "name");
                selectedWidth = util::parseUserInt(_widthText->data, "width", 1, 1000);
                selectedHeight = util::parseUserInt(_heightText->data, "height", 1, 1000);
                selectedName = _nameText->data;
            } catch (std::runtime_error& ex) {
                messageBox(ex.what(), mfError | mfOKButton);
                clearEvent(event);
            }
        }

        TDialog::handleEvent(event);
    }

   private:
    ViewPtr<InputLine> _nameText;
    ViewPtr<InputLine> _widthText;
    ViewPtr<InputLine> _heightText;
};

enum class PictureWindowMode {
    // tools
    kSelect,
    kDraw,
    kPick,
    kType,
    kMask,

    // operations
    kPaste,
    kMove
};

class PictureWindowPrivate {
   public:
    explicit PictureWindowPrivate(turbo::Clipboard* aClipboard);
    void updateScrollBars();
    void enableDisableCommands(bool enable);
    void updateStatusItems();
    void onClear();
    void onCut();
    void onCopy();
    void onPaste();
    void onPasteCancel();
    void onPasteOk();
    void onMove();
    void onMoveCancel();
    void onMoveOk();
    void onMouse(int pictureX, int pictureY, const CanvasViewMouseEventArgs& e);
    void onTick();
    void checkpoint();
    void onUndo();
    void onRedo();

    int ticks = 0;
    compiler::SourceMember* member{};
    std::function<void()> onEdited;
    PictureWindowStatusItems statusItems;
    turbo::Clipboard* clipboard;

    // shared state
    TColorRGB fg{ 255, 255, 255 };
    TColorRGB bg{ 0, 0, 255 };
    std::string ch = "♫";
    PictureWindowMode mode = PictureWindowMode::kDraw;
    UndoStack<Picture> undoStack{};

    // select and type tool
    std::optional<TPoint> currentDrag;

    // select tool > paste operation
    std::optional<TPoint> currentPasteDrag;                 // the location where the user started dragging
    std::optional<TPoint> currentPasteDragPictureLocation;  // pos. of the picture when the user started dragging

    // select tool > move operation
    std::optional<TPoint> currentMoveDrag;                   // the location where the user started dragging
    std::optional<TPoint> currentMoveDragSelectionLocation;  // pos. of the selection when the user started dragging

    // mask tool
    bool currentDragTransparent = false;

    // drag resizing
    bool resizing = false;
    TPoint resizingStartPicturePoint{};
    std::optional<Picture> resizingOriginalPicture;
    bool resizingVertical = false;
    bool resizingHorizontal = false;

    // shared GUI
    ViewPtr<CanvasView> canvasView{ TRect() };
    ViewPtr<ScrollBar> vScrollBar{ TRect(0, 0, 1, 10) };
    ViewPtr<ScrollBar> hScrollBar{ TRect(0, 0, 10, 1) };
    ViewPtr<Label> toolLabel{ TRect(1, 1, 10, 2) };
    ViewPtr<CheckBoxes> setFgCheck{ std::vector<std::string>{ "Set F~G~" }, std::vector<bool>{ true } };
    ViewPtr<CheckBoxes> setBgCheck{ std::vector<std::string>{ "Set ~B~G" }, std::vector<bool>{ true } };
    ViewPtr<CheckBoxes> setChCheck{ std::vector<std::string>{ "Set ~c~har" }, std::vector<bool>{ true } };

    // mask tool
    ViewPtr<Label> maskHelp{ "Click to toggle between opaque and transparent." };

    // select tool
    ViewPtr<ThinButton> cutButton{ "Cut", cmCut };
    ViewPtr<ThinButton> copyButton{ "Copy", cmCopy };
    ViewPtr<ThinButton> pasteButton{ "Paste", cmPaste };
    ViewPtr<ThinButton> clearButton{ "Delete", cmClear };
    ViewPtr<ThinButton> moveButton{ "~M~ove", kCmdPictureMove };

    // select tool -> paste operation
    ViewPtr<Label> pasteHelp{ "Drag or use arrow keys to move." };
    ViewPtr<ThinButton> pasteOkButton{ "OK", kCmdPicturePasteOk };
    ViewPtr<ThinButton> pasteCancelButton{ "Cancel", kCmdPicturePasteCancel };

    // select tool -> move operation
    ViewPtr<Label> moveHelp{ "Drag or use arrow keys to move." };
    ViewPtr<ThinButton> moveOkButton{ "OK", kCmdPictureMoveOk };
    ViewPtr<ThinButton> moveCancelButton{ "Cancel", kCmdPictureMoveCancel };
};

static std::string getPictureWindowTitle(const std::string& name) {
    return name + " (Picture)";
}

PictureWindow::PictureWindow(
    const TRect& r,
    turbo::Clipboard* clipboard,
    SourceMember* member,
    std::function<void()> onEdited,
    const PictureWindowStatusItems& statusItems)
    : TWindow(r, getPictureWindowTitle(member->identifier), wnNoNumber),
      TWindowInit(TWindow::initFrame),
      _private(new PictureWindowPrivate(clipboard)) {
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
            _private->moveButton.take(),
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

    RowLayout(
        false,
        {
            _private->moveOkButton.take(),
            _private->moveCancelButton.take(),
            _private->moveHelp.take(),
        })
        .addTo(this, 10, 80, 1);
    _private->moveHelp->hide();
    _private->moveOkButton->hide();
    _private->moveCancelButton->hide();

    try {
        _private->canvasView->picture = Picture(member->source);
    } catch (...) {
    }

    _private->canvasView->setBounds(TRect(1, 2, size.x - 1, size.y - 1));
    _private->canvasView->growMode = gfGrowHiX | gfGrowHiY;
    _private->canvasView.addTo(this);
    _private->canvasView->select();

    _private->updateScrollBars();
}

PictureWindow::~PictureWindow() {
    delete _private;
}

PictureWindowPrivate::PictureWindowPrivate(turbo::Clipboard* aClipboard) : clipboard(aClipboard) {}

void PictureWindowPrivate::updateScrollBars() {
    vScrollBar->setParams(
        vScrollBar->value, 0, std::max(0, canvasView->picture.height + 5 - (canvasView->size.y - 1)),
        canvasView->size.y - 1, 1);
    hScrollBar->setParams(
        hScrollBar->value, 0, std::max(0, canvasView->picture.width + 10 - (canvasView->size.x - 1)),
        canvasView->size.x - 1, 1);
}

void PictureWindowPrivate::enableDisableCommands(bool enable) {
    util::enableDisableCommands(
        enable,
        {
            kCmdPictureCharacter,
            kCmdPictureFg,
            kCmdPictureBg,
            kCmdPictureSelect,
            kCmdPictureDraw,
            kCmdPicturePick,
            kCmdPictureType,
            kCmdPictureMask,
            kCmdPictureOptions,
            cmPaste,
            cmClear,
        });

    util::enableDisableCommands(
        enable && canvasView->selection.has_value(),
        {
            cmCut,
            cmCopy,
            kCmdPictureMove,
        });

    util::enableDisableCommands(
        enable && mode == PictureWindowMode::kPaste,
        {
            kCmdPicturePasteOk,
            kCmdPicturePasteCancel,
        });

    util::enableDisableCommands(
        enable && mode == PictureWindowMode::kMove,
        {
            kCmdPictureMoveOk,
            kCmdPictureMoveCancel,
        });

    util::enableDisableCommand(enable && undoStack.canUndo(), cmUndo);
    util::enableDisableCommand(enable && undoStack.canRedo(), kCmdEditRedo);
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

void PictureWindowPrivate::updateStatusItems() {
    std::ostringstream chText;
    chText << "~F3~ " << ch;
    delete[] statusItems.character->text;  // NOLINT
    statusItems.character->text = newStr(chText.str());

    TAttrPair attrPair{ TColorAttr(fg, bg), TColorAttr(fg, bg) };
    statusItems.fgColor->colorPairNormal = attrPair;
    statusItems.bgColor->colorPairNormal = attrPair;
    statusItems.characterColor->colorPairNormal = attrPair;

    TAttrPair sel{ 0x20, 0x2E };
    TAttrPair unsel{ 0x70, 0x74 };

    statusItems.selectColor->colorPairNormal = mode == PictureWindowMode::kSelect ? sel : unsel;
    statusItems.drawColor->colorPairNormal = mode == PictureWindowMode::kDraw ? sel : unsel;
    statusItems.pickColor->colorPairNormal = mode == PictureWindowMode::kPick ? sel : unsel;
    statusItems.textColor->colorPairNormal = mode == PictureWindowMode::kType ? sel : unsel;
    statusItems.maskColor->colorPairNormal = mode == PictureWindowMode::kMask ? sel : unsel;

    StatusLine::setItemText(statusItems.select, mode == PictureWindowMode::kSelect ? "―► Select" : "~F4~ Select");
    StatusLine::setItemText(statusItems.draw, mode == PictureWindowMode::kDraw ? "―► Draw" : "~F5~ Draw");
    StatusLine::setItemText(statusItems.pick, mode == PictureWindowMode::kPick ? "―► Pick" : "~F6~ Pick");
    StatusLine::setItemText(statusItems.text, mode == PictureWindowMode::kType ? "―► Type" : "~F7~ Type");
    StatusLine::setItemText(statusItems.mask, mode == PictureWindowMode::kMask ? "―► Mask" : "~F8~ Mask");

    statusItems.statusLine->drawView();

    std::string labelText;
    switch (mode) {
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
        case PictureWindowMode::kMove:
            labelText = "Move";
            break;
        default:
            assert(false);
            break;
    }

    // Copy, Move, Fill
    showHide(mode == PictureWindowMode::kSelect, { copyButton, cutButton, pasteButton, clearButton, moveButton });

    // Set FG, Set BG
    showHide(
        mode == PictureWindowMode::kDraw || mode == PictureWindowMode::kType || mode == PictureWindowMode::kPick,
        { setBgCheck, setFgCheck });

    // Set Char
    showHide(mode == PictureWindowMode::kDraw || mode == PictureWindowMode::kPick, { setChCheck });

    // Selection
    if (canvasView->selection.has_value()) {
        auto sel = *canvasView->selection;
        if (mode != PictureWindowMode::kSelect && mode != PictureWindowMode::kType) {
            canvasView->selection = {};
            enableDisableCommands(true);
            canvasView->drawView();
        }
        if (mode == PictureWindowMode::kType && (sel.b.x > sel.a.x + 1 || sel.b.y > sel.a.y + 1)) {
            canvasView->selection = TRect(sel.a.x, sel.a.y, sel.a.x + 1, sel.a.y + 1);
            enableDisableCommands(true);
            canvasView->drawView();
        }
    }

    // Paste
    showHide(mode == PictureWindowMode::kPaste, { pasteHelp, pasteOkButton, pasteCancelButton });

    // Selection > Move
    showHide(mode == PictureWindowMode::kMove, { moveHelp, moveOkButton, moveCancelButton });

    // Mask help
    showHide(mode == PictureWindowMode::kMask, { maskHelp });

    toolLabel->setTitle(labelText);
    toolLabel->drawView();
}

void PictureWindowPrivate::onClear() {
    checkpoint();
    PictureCell pictureCell{ false, { fg, bg }, " " };

    auto& picture = canvasView->picture;
    const auto selection =
        canvasView->selection.has_value() ? *canvasView->selection : TRect{ 0, 0, picture.width, picture.height };
    for (auto y = selection.a.y; y < selection.b.y; y++) {
        for (auto x = selection.a.x; x < selection.b.x; x++) {
            picture.cells.at(y * picture.width + x) = pictureCell;
        }
    }

    canvasView->selection = {};
    enableDisableCommands(true);
    canvasView->drawView();
}

void PictureWindowPrivate::onCut() {
    _clipboardText = canvasView->getSelectionTextForClipboard();
    clipboard->setText(_clipboardText.text);
    checkpoint();
    onClear();
}

void PictureWindowPrivate::onCopy() {
    _clipboardText = canvasView->getSelectionTextForClipboard();
    clipboard->setText(_clipboardText.text);
}

void PictureWindowPrivate::onPaste() {
    ClipboardText ct{};
    clipboard->getText([&](TStringView text) { ct.text = text; });

    if (ct.text.empty()) {
        return;
    }

    checkpoint();

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

    canvasView->pastedPicture = { maxLineLength, static_cast<int>(charsByRowThenColumn.size()) };
    canvasView->pastedPictureLocation = { 0, 0 };
    auto& pastedPicture = *canvasView->pastedPicture;
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

    mode = PictureWindowMode::kPaste;
    enableDisableCommands(true);
    updateStatusItems();
    canvasView->drawView();
}

void PictureWindowPrivate::onPasteCancel() {
    currentPasteDrag = {};
    currentPasteDragPictureLocation = {};
    canvasView->pastedPicture = {};
    canvasView->pastedPictureLocation = {};
    canvasView->drawView();
    mode = PictureWindowMode::kSelect;
    updateStatusItems();
    enableDisableCommands(true);
}

void PictureWindowPrivate::onPasteOk() {
    checkpoint();
    canvasView->commitPaste();
    canvasView->drawView();
    onPasteCancel();
}

void PictureWindowPrivate::onMove() {
    if (!canvasView->selection.has_value()) {
        return;
    }

    canvasView->pastedPicture = canvasView->picture.crop(*canvasView->selection);
    canvasView->pastedPictureLocation = canvasView->selection->a;
    canvasView->moveOriginalColor = { fg, bg };
    canvasView->moveOriginalRect = canvasView->selection;
    mode = PictureWindowMode::kMove;
    updateStatusItems();
    enableDisableCommands(true);
}

void PictureWindowPrivate::onMoveCancel() {
    currentMoveDrag = {};
    currentMoveDragSelectionLocation = {};
    canvasView->pastedPicture = {};
    canvasView->pastedPictureLocation = {};
    canvasView->moveOriginalColor = {};
    canvasView->moveOriginalRect = {};
    canvasView->drawView();
    mode = PictureWindowMode::kSelect;
    updateStatusItems();
    enableDisableCommands(true);
}

void PictureWindowPrivate::onMoveOk() {
    checkpoint();
    canvasView->commitPaste();
    canvasView->drawView();
    onMoveCancel();
}

void PictureWindowPrivate::checkpoint() {
    undoStack.push(canvasView->picture);
    onEdited();
    enableDisableCommands(true);
}

void PictureWindowPrivate::onUndo() {
    canvasView->picture = undoStack.undo(std::move(canvasView->picture));
    canvasView->drawView();
    if (mode == PictureWindowMode::kPaste) {
        onPasteCancel();
    }
    if (mode == PictureWindowMode::kMove) {
        onMoveCancel();
    }
    enableDisableCommands(true);
}

void PictureWindowPrivate::onRedo() {
    canvasView->picture = undoStack.redo(std::move(canvasView->picture));
    canvasView->drawView();
    if (mode == PictureWindowMode::kPaste) {
        onPasteCancel();
    }
    if (mode == PictureWindowMode::kMove) {
        onMoveCancel();
    }
    enableDisableCommands(true);
}

void PictureWindowPrivate::onMouse(int pictureX, int pictureY, const CanvasViewMouseEventArgs& e) {
    TPoint pt{ pictureX, pictureY };
    auto& picture = canvasView->picture;
    auto leftMouseDown = e.down && (e.buttons & mbLeftButton) != 0;
    auto rightMouseDown = e.down && (e.buttons & mbRightButton) != 0;
    auto leftDragging = e.move && (e.buttons & mbLeftButton) != 0;
    auto rightDragging = e.move && (e.buttons & mbRightButton) != 0;

    TPoint verticalGripper{ picture.width / 2, picture.height };
    TPoint horizontalGripper{ picture.width + 1, picture.height / 2 };
    TPoint diagonalGripper{ picture.width + 1, picture.height };
    if (leftMouseDown && (pt == verticalGripper || pt == horizontalGripper || pt == diagonalGripper)) {
        // the user has started to drag the resize gripper
        checkpoint();
        resizing = true;
        resizingStartPicturePoint = pt;
        resizingOriginalPicture = picture;
        resizingVertical = pt == verticalGripper || pt == diagonalGripper;
        resizingHorizontal = pt == horizontalGripper || pt == diagonalGripper;
        canvasView->selection = {};
        enableDisableCommands(true);
        canvasView->drawView();
        return;
    }
    if (leftDragging && resizing) {
        // the user continues to drag the resize gripper
        auto deltaX = 0;
        if (resizingHorizontal) {
            deltaX = pt.x - resizingStartPicturePoint.x;
        }
        auto deltaY = 0;
        if (resizingVertical) {
            deltaY = pt.y - resizingStartPicturePoint.y;
        }
        auto newWidth = resizingOriginalPicture->width + deltaX;
        auto newHeight = resizingOriginalPicture->height + deltaY;
        canvasView->picture = *resizingOriginalPicture;
        canvasView->picture.resize(newWidth, newHeight);
        canvasView->selection = {};
        enableDisableCommands(true);
        canvasView->drawView();
        return;
    }
    if (e.up && resizing) {
        // the user has stopped dragging the resize gripper
        if (canvasView->picture.width == resizingOriginalPicture->width &&
            canvasView->picture.height == resizingOriginalPicture->height) {
            // the user didn't actually change the size of the picture so don't create an undo record for this
            undoStack.abandon();
        }
        resizing = false;
        resizingOriginalPicture = {};
        updateScrollBars();
        return;
    }

    // paste handled earlier here because in this mode, the user may click outside the picture
    if (mode == PictureWindowMode::kPaste) {
        if (currentPasteDrag.has_value()) {
            // the user is continuing to drag the pasted picture
            auto xOffset = pictureX - currentPasteDrag->x;
            auto yOffset = pictureY - currentPasteDrag->y;
            auto xNew = currentPasteDragPictureLocation->x + xOffset;
            auto yNew = currentPasteDragPictureLocation->y + yOffset;
            canvasView->pastedPictureLocation = { xNew, yNew };
        } else if (e.down) {
            // if this is inside the pasted picture, then the user has started to drag it
            // if this is outside the pasted picture, then the user is accepting the paste
            auto& loc = canvasView->pastedPictureLocation;
            if (pictureX >= loc->x && pictureY >= loc->y && pictureX < loc->x + canvasView->pastedPicture->width &&
                pictureY < loc->y + canvasView->pastedPicture->height) {
                currentPasteDrag = { pictureX, pictureY };
                currentPasteDragPictureLocation = canvasView->pastedPictureLocation;
            } else {
                onPasteOk();
                updateStatusItems();
            }
        }
        if ((e.up || (e.move && e.buttons == 0)) && currentPasteDrag.has_value()) {
            // the user has stopped dragging the pasted picture
            currentPasteDrag = {};
            currentPasteDragPictureLocation = {};
        }
        canvasView->drawView();
        return;
    }

    // move handled earlier here because in this mode, the user may click outside the picture
    if (mode == PictureWindowMode::kMove) {
        if (currentMoveDrag.has_value()) {
            // the user is continuing to drag-move a selection
            auto xOffset = pictureX - currentMoveDrag->x;
            auto yOffset = pictureY - currentMoveDrag->y;
            auto xNew = currentMoveDragSelectionLocation->x + xOffset;
            auto yNew = currentMoveDragSelectionLocation->y + yOffset;
            auto w = canvasView->selection->b.x - canvasView->selection->a.x;
            auto h = canvasView->selection->b.y - canvasView->selection->a.y;
            canvasView->selection = TRect{ xNew, yNew, xNew + w, yNew + h };
            canvasView->pastedPictureLocation = canvasView->selection->a;
        } else if (e.down) {
            // if this is inside the selection, then the user has started to drag it
            // if this is outside the selection, then the user is accepting the move
            auto& loc = canvasView->pastedPictureLocation;
            if (pictureX >= loc->x && pictureY >= loc->y && pictureX < loc->x + canvasView->pastedPicture->width &&
                pictureY < loc->y + canvasView->pastedPicture->height) {
                currentMoveDrag = { pictureX, pictureY };
                currentMoveDragSelectionLocation = canvasView->pastedPictureLocation;
            } else {
                onMoveOk();
            }
        }
        if ((e.up || (e.move && e.buttons == 0)) && currentMoveDrag.has_value()) {
            // the user has stopped dragging the selection
            currentMoveDrag = {};
            currentMoveDragSelectionLocation = {};
        }
        canvasView->drawView();
        return;
    }

    if (pt.x < 0 || pt.x >= picture.width || pt.y < 0 || pt.y >= picture.height) {
        // the user clicked outside the picture
        return;
    }

    auto& cell = picture.cells.at(pictureY * picture.width + pictureX);
    TColorAttr color{ fg, bg };

    switch (mode) {
        case PictureWindowMode::kSelect:
            if (currentDrag.has_value()) {
                // the user is continuing to drag-select
                auto x1 = currentDrag->x;
                auto y1 = currentDrag->y;
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
                canvasView->selection = TRect(x1, y1, x2, y2);
                enableDisableCommands(true);
            } else if (e.down) {
                // the user has started some kind of drag operation
                if (canvasView->selection.has_value() &&
                    canvasView->selection->contains(TPoint{ pictureX, pictureY })) {
                    // the user has started drag-moving
                    onMove();
                    currentMoveDrag = { pictureX, pictureY };
                    currentMoveDragSelectionLocation = canvasView->pastedPictureLocation;
                } else {
                    // the user has started drag-selecting
                    currentDrag = { pictureX, pictureY };
                    canvasView->selection = TRect(*currentDrag, TPoint{ currentDrag->x + 1, currentDrag->y + 1 });
                    enableDisableCommands(true);
                }
            }
            if ((e.up || (e.move && e.buttons == 0)) && currentDrag.has_value()) {
                // the user has stopped drag-selecting
                currentDrag = {};
            }
            canvasView->drawView();
            break;

        case PictureWindowMode::kDraw:
            if (leftMouseDown || rightMouseDown) {
                // the user started drawing
                checkpoint();
            }
            if (leftMouseDown || rightMouseDown || leftDragging || rightDragging) {
                // the user is continuing to draw
                auto right = rightMouseDown || rightDragging;
                if (setFgCheck->mark(0)) {
                    cell.colorAttr._fg = right ? color._bg : color._fg;
                    cell.colorAttr._style = color._style;
                }
                if (setBgCheck->mark(0)) {
                    cell.colorAttr._bg = right ? color._fg : color._bg;
                }
                if (setChCheck->mark(0)) {
                    cell.ch = ch;
                }
                cell.transparent = false;
                canvasView->drawView();
            }
            break;

        case PictureWindowMode::kPick:
            if (leftMouseDown || rightMouseDown) {
                // the user started picking
                checkpoint();
            }
            if (leftMouseDown || rightMouseDown || leftDragging || rightDragging) {
                // the user is continuing to pick
                auto right = rightMouseDown || rightDragging;
                if (setBgCheck->mark(0)) {
                    bg = right ? getFore(cell.colorAttr).asRGB() : getBack(cell.colorAttr).asRGB();
                }
                if (setFgCheck->mark(0)) {
                    fg = right ? getBack(cell.colorAttr).asRGB() : getFore(cell.colorAttr).asRGB();
                }
                if (setChCheck->mark(0)) {
                    ch = cell.ch;
                }
                updateStatusItems();
            }
            break;

        case PictureWindowMode::kType:
            if (leftMouseDown || leftDragging) {
                // the user is clicking to move the cursor in type mode
                canvasView->selection = TRect(pictureX, pictureY, pictureX + 1, pictureY + 1);
                enableDisableCommands(true);
                canvasView->drawView();
            }
            break;

        case PictureWindowMode::kMask:
            if (leftMouseDown) {
                // the user started drawing in mask mode
                checkpoint();
                currentDragTransparent = !cell.transparent;
            }
            if (leftMouseDown || leftDragging) {
                // the user is continuing to draw in mask mode
                cell.transparent = currentDragTransparent;
                canvasView->drawView();
            }
            break;

        default:
            break;
    }
}

void PictureWindowPrivate::onTick() {
    // blink the selection rectangle
    // blink the transparent cells if we're in mask mode
    ticks++;
    if (ticks >= 2) {
        if (mode == PictureWindowMode::kMask) {
            canvasView->flashingMask = !canvasView->flashingMask;
            canvasView->drawView();
        } else if (
            mode == PictureWindowMode::kSelect || mode == PictureWindowMode::kType ||
            mode == PictureWindowMode::kPaste || mode == PictureWindowMode::kMove) {
            canvasView->flashingSelection = !canvasView->flashingSelection;
            canvasView->drawView();
        }
        ticks = 0;
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
            _private->canvasView->selection = {};
            _private->enableDisableCommands(true);
            _private->canvasView->drawView();
            clearEvent(event);
        } else if (event.keyDown.keyCode == kbEsc && _private->mode == PictureWindowMode::kPaste) {
            // user is pressing ESC to cancel a paste
            _private->onPasteCancel();
            clearEvent(event);
        } else if (event.keyDown.keyCode == kbEnter && _private->mode == PictureWindowMode::kPaste) {
            // user is pressing Enter to confirm a paste
            _private->onPasteOk();
            clearEvent(event);
        } else if (event.keyDown.keyCode == kbEsc && _private->mode == PictureWindowMode::kMove) {
            // user is pressing ESC to cancel a move
            _private->onMoveCancel();
            clearEvent(event);
        } else if (event.keyDown.keyCode == kbEnter && _private->mode == PictureWindowMode::kMove) {
            // user is pressing Enter to confirm a move
            _private->onMoveOk();
            clearEvent(event);
        } else if (
            _private->mode == PictureWindowMode::kType && _private->canvasView->selection.has_value() &&
            event.keyDown.keyCode == kbBack) {
            // user is pressing Backspace in type mode
            auto width = _private->canvasView->picture.width;
            auto height = _private->canvasView->picture.height;
            auto r = *_private->canvasView->selection;
            r.a.x -= 1;
            r.b.x -= 1;
            if (r.a.x >= 0 && r.a.x <= width && r.b.x >= 0 && r.b.x <= width && r.a.y >= 0 && r.a.y <= height &&
                r.b.y >= 0 && r.b.y <= height) {
                auto& cell =
                    _private->canvasView->picture.cells.at(r.a.y * _private->canvasView->picture.width + r.a.x);
                cell.ch = ' ';
                _private->canvasView->selection = r;
                _private->enableDisableCommands(true);
                _private->canvasView->drawView();
            }
        } else if (
            _private->mode == PictureWindowMode::kType && _private->canvasView->selection.has_value() &&
            event.keyDown.keyCode == kbDel) {
            // user is pressing Delete in type mode
            auto r = *_private->canvasView->selection;
            auto& cell = _private->canvasView->picture.cells.at(r.a.y * _private->canvasView->picture.width + r.a.x);
            cell.ch = ' ';
            _private->canvasView->drawView();
        } else if (
            _private->mode == PictureWindowMode::kType && _private->canvasView->selection.has_value() &&
            event.keyDown.text[0] != '\0' && event.keyDown.charScan.charCode != 0) {
            // user is typing text in type mode
            _private->checkpoint();
            auto rect = *_private->canvasView->selection;
            auto& cell =
                _private->canvasView->picture.cells.at(rect.a.y * _private->canvasView->picture.width + rect.a.x);
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

            if (rect.a.x < _private->canvasView->picture.width - 1) {
                rect.a.x++;
                rect.b.x++;
            }
            _private->canvasView->selection = rect;
            _private->enableDisableCommands(true);
            _private->canvasView->drawView();
            clearEvent(event);
        } else if (
            (_private->mode == PictureWindowMode::kSelect || _private->mode == PictureWindowMode::kType) &&
            (event.keyDown.keyCode == kbLeft || event.keyDown.keyCode == kbRight || event.keyDown.keyCode == kbUp ||
             event.keyDown.keyCode == kbDown) &&
            // user is using the arrow keys to move the cursor in select or type mode
            _private->canvasView->selection.has_value()) {
            auto deltaX = 0;
            auto deltaY = 0;
            getArrowKeyDirection(event.keyDown.keyCode, &deltaX, &deltaY);
            auto width = _private->canvasView->picture.width;
            auto height = _private->canvasView->picture.height;
            auto r = *_private->canvasView->selection;
            r.a.x += deltaX;
            r.b.x += deltaX;
            r.a.y += deltaY;
            r.b.y += deltaY;
            if (r.a.x >= 0 && r.a.x <= width && r.b.x >= 0 && r.b.x <= width && r.a.y >= 0 && r.a.y <= height &&
                r.b.y >= 0 && r.b.y <= height) {
                _private->canvasView->selection = r;
                _private->enableDisableCommands(true);
                _private->canvasView->drawView();
            }
        } else if (
            (_private->mode == PictureWindowMode::kPaste || _private->mode == PictureWindowMode::kMove) &&
            (event.keyDown.keyCode == kbLeft || event.keyDown.keyCode == kbRight || event.keyDown.keyCode == kbUp ||
             event.keyDown.keyCode == kbDown)) {
            // user is using the arrow keys to move the pasted picture in paste or move mode
            auto deltaX = 0;
            auto deltaY = 0;
            getArrowKeyDirection(event.keyDown.keyCode, &deltaX, &deltaY);
            auto width = _private->canvasView->picture.width;
            auto height = _private->canvasView->picture.height;
            auto p = *_private->canvasView->pastedPictureLocation;
            p.x += deltaX;
            p.y += deltaY;
            if (p.x >= 0 && p.x < width && p.y >= 0 && p.y < height) {
                _private->canvasView->pastedPictureLocation = p;
                _private->canvasView->drawView();
            }
        }
    }

    TWindow::handleEvent(event);

    if (event.what == evCommand) {
        switch (event.message.command) {
            case kCmdPictureViewMouse: {
                auto* e = reinterpret_cast<CanvasViewMouseEventArgs*>(event.message.infoPtr);  // NOLINT
                auto pictureX = _private->canvasView->viewXToPicture(e->viewPoint.x);
                auto pictureY = _private->canvasView->viewYToPicture(e->viewPoint.y);
                _private->onMouse(pictureX, pictureY, *e);
                break;
            }

            case cmClear:
                _private->onClear();
                clearEvent(event);
                break;

            case cmCut:
                if (_private->mode == PictureWindowMode::kSelect) {
                    _private->onCut();
                }
                clearEvent(event);
                break;

            case cmCopy:
                if (_private->mode == PictureWindowMode::kSelect) {
                    _private->onCopy();
                }
                clearEvent(event);
                break;

            case cmPaste:
                if (_private->mode == PictureWindowMode::kPaste) {
                    _private->onPasteOk();
                }
                _private->onPaste();
                clearEvent(event);
                break;

            case kCmdPicturePasteCancel:
                _private->onPasteCancel();
                clearEvent(event);
                break;

            case kCmdPicturePasteOk:
                _private->onPasteOk();
                clearEvent(event);
                break;

            case kCmdPictureMove:
                if (_private->mode == PictureWindowMode::kSelect) {
                    _private->onMove();
                }
                clearEvent(event);
                break;

            case kCmdPictureMoveCancel:
                _private->onMoveCancel();
                clearEvent(event);
                break;

            case kCmdPictureMoveOk:
                _private->onMoveOk();
                clearEvent(event);
                break;

            case cmUndo:
                _private->onUndo();
                clearEvent(event);
                break;

            case kCmdEditRedo:
                _private->onRedo();
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
                _private->canvasView->scrollTop = _private->vScrollBar->value;
                _private->canvasView->scrollLeft = _private->hScrollBar->value;
                _private->canvasView->drawView();
                break;

            case kCmdTimerTick:
                _private->onTick();
                break;
        }
    }
}

uint16_t PictureWindow::getHelpCtx() {
    return hcide_pictureWindow;
}

void PictureWindow::close() {
    auto newSource = _private->canvasView->picture.exportToString();
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
            _private->updateStatusItems();
        }
        dynamic_cast<App&>(*TProgram::application).setPictureWindow(enable ? this : nullptr);

        _private->enableDisableCommands(enable);
    }
}

void PictureWindow::changeBounds(const TRect& bounds) {
    TWindow::changeBounds(bounds);
    _private->updateScrollBars();
}

void PictureWindow::onStatusLineCommand(ushort cmd) {
    auto previousMode = _private->mode;

    if (cmd != kCmdPictureMask) {
        _private->canvasView->flashingMask = false;
        _private->canvasView->drawView();
    }

    switch (cmd) {
        case kCmdPictureFg: {
            TColorRGB selection{};
            if (InsertColorDialog::go("Choose Foreground Color", "Choose", &selection)) {
                _private->fg = selection;
                _private->updateStatusItems();
            }
            break;
        }

        case kCmdPictureBg: {
            TColorRGB selection{};
            if (InsertColorDialog::go("Choose Background Color", "Choose", &selection)) {
                _private->bg = selection;
                _private->updateStatusItems();
            }
            break;
        }

        case kCmdPictureCharacter: {
            auto dialog = DialogPtr<InsertSymbolDialog>("Choose Character", "Choose");
            if (TProgram::deskTop->execView(dialog) == cmOK) {
                _private->ch = dialog->getSelection();
                _private->updateStatusItems();
            }
            break;
        }

        case kCmdPictureSelect:
            _private->mode = PictureWindowMode::kSelect;
            _private->updateStatusItems();
            break;

        case kCmdPictureDraw:
            _private->mode = PictureWindowMode::kDraw;
            _private->updateStatusItems();
            break;

        case kCmdPicturePick:
            _private->mode = PictureWindowMode::kPick;
            _private->updateStatusItems();
            break;

        case kCmdPictureType:
            _private->mode = PictureWindowMode::kType;
            if (!_private->canvasView->selection.has_value() && _private->canvasView->picture.width > 0 &&
                _private->canvasView->picture.height > 0) {
                _private->canvasView->selection = { 0, 0, 1, 1 };
                _private->enableDisableCommands(true);
                _private->canvasView->drawView();
            }
            _private->updateStatusItems();
            break;

        case kCmdPictureMask:
            _private->mode = PictureWindowMode::kMask;
            _private->updateStatusItems();
            break;

        case kCmdPictureOptions: {
            DialogPtr<PictureOptionsDialog> dialog{ &_private->canvasView->picture };
            if (TProgram::deskTop->execView(dialog) == cmOK) {
                bool edited = false;
                auto& pic = _private->canvasView->picture;
                if (pic.name != dialog->selectedName) {
                    pic.name = dialog->selectedName;
                    delete[] title;
                    title = newStr(getPictureWindowTitle(_private->canvasView->picture.name));
                    frame->drawView();
                    _private->member->setSource(_private->canvasView->picture.exportToString());
                    _private->onEdited();
                    edited = true;
                }

                if (pic.width != dialog->selectedWidth || pic.height != dialog->selectedHeight) {
                    _private->checkpoint();
                    _private->canvasView->picture.resize(dialog->selectedWidth, dialog->selectedHeight);
                    _private->canvasView->drawView();
                    _private->updateScrollBars();
                    edited = true;
                }

                if (edited) {
                    _private->onEdited();
                }
            }
            break;
        }

        default:
            break;
    }

    if (previousMode == PictureWindowMode::kPaste && _private->mode != PictureWindowMode::kPaste) {
        _private->onPasteOk();
    }
    if (previousMode == PictureWindowMode::kMove && _private->mode != PictureWindowMode::kMove) {
        _private->onMoveOk();
    }
}

}  // namespace tmbasic
