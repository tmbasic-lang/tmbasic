#include "ProgramWindow.h"
#include "shared/vm/Program.h"
#include "constants.h"

namespace tmbasic {

enum class ProgramItemType { kProcedure, kGlobal, kType };

static const char* kProgramItemTypeStrings[] = {
    "Procedures \x10",
    "Globals    \x10",
    "Types      \x10",
};

class ProgramItemTypesListBox : public TListViewer {
   public:
    ProgramItemTypesListBox(const TRect& bounds, ushort numCols, TScrollBar* vScrollBar)
        : TListViewer(bounds, numCols, nullptr, vScrollBar) {
        setRange(3);
    }

    virtual ~ProgramItemTypesListBox() {}

    void getText(char* dest, short item, short maxLen) override {
        strncpy(dest, kProgramItemTypeStrings[item], maxLen);
        dest[maxLen] = '\0';
    }

    void focusItem(short item) override {
        TListViewer::focusItem(item);
        message(owner, evBroadcast, kCmdProgramItemTypeSelected, nullptr);
    }

    TPalette& getPalette() const override {
        // Active, Inactive, Focused, Selected, Divider
        static const char bytes[] = kWindowPaletteFrameActive kWindowPaletteFramePassive
            kWindowPaletteScrollerSelectedText kWindowPaletteScrollBarControls kWindowPaletteFrameActive;
        static auto palette = TPalette(bytes, sizeof(bytes) - 1);
        return palette;
    }
};

class ProgramItem {
   public:
    ProgramItemType type;
    size_t index;
    std::string name;

    ProgramItem(ProgramItemType type, size_t index, std::string name) : type(type), index(index), name(name) {}
};

class ProgramContentsListBox : public TListViewer {
   private:
    std::vector<ProgramItem> _items;

   public:
    ProgramContentsListBox(const TRect& bounds, ushort numCols, TScrollBar* vScrollBar)
        : TListViewer(bounds, numCols, nullptr, vScrollBar) {
        setRange(0);
    }

    virtual ~ProgramContentsListBox() {}

    void setData(std::vector<ProgramItem> items) {
        _items = std::move(items);
        setRange(_items.size());
        drawView();
    }

    void getText(char* dest, short item, short maxLen) override {
        if (item >= 0 && static_cast<size_t>(item) < _items.size()) {
            strncpy(dest, _items[item].name.c_str(), maxLen);
            dest[maxLen] = '\0';
        } else {
            dest[0] = '\0';
        }
    }

    void handleEvent(TEvent& event) override {
        TListViewer::handleEvent(event);
        if (event.what == evKeyDown && event.keyDown.keyCode == kbEnter) {
            // TODO - 'focused' is the selected index value
        }
    }

    TPalette& getPalette() const override {
        // Active, Inactive, Focused, Selected, Divider
        static const char bytes[] = kWindowPaletteFrameActive kWindowPaletteFramePassive
            kWindowPaletteScrollerSelectedText kWindowPaletteScrollBarControls kWindowPaletteFrameActive;
        static auto palette = TPalette(bytes, sizeof(bytes) - 1);
        return palette;
    }
};

class ProgramWindow : public TWindow {
   public:
    ProgramWindow(const TRect& r) : TWindow(r, "Untitled (program)", wnNoNumber), TWindowInit(TWindow::initFrame) {
        palette = 0;
        auto* vScrollBar = new TScrollBar(TRect(size.x - 1, 1, size.x, size.y - 1));
        insert(vScrollBar);

        auto typesListBoxRect = getExtent();
        typesListBoxRect.grow(-1, -1);
        typesListBoxRect.b.x = 15;
        _typesListBox = new ProgramItemTypesListBox(typesListBoxRect, 1, nullptr);
        _typesListBox->growMode = gfGrowHiY;
        insert(_typesListBox);

        auto contentsListBoxRect = getExtent();
        contentsListBoxRect.grow(-1, -1);
        contentsListBoxRect.a.x = 16;
        _contentsListBox = new ProgramContentsListBox(contentsListBoxRect, 1, vScrollBar);
        _contentsListBox->growMode = gfGrowHiX | gfGrowHiY;
        insert(_contentsListBox);

        auto v = std::vector<ProgramItem>();
        // v.push_back(ProgramItem(ProgramItemType::kGlobal, 0, "(Globals)"));
        _contentsListBox->setData(std::move(v));
    }

    virtual ~ProgramWindow() {}

    TPalette& getPalette() const override {
        static auto palette = TPalette(cpBlueWindow, sizeof(cpBlueWindow) - 1);
        return palette;
    }

   private:
    ProgramItemTypesListBox* _typesListBox;
    ProgramContentsListBox* _contentsListBox;
};

TWindow* newProgramWindow(const TRect& r) {
    return new ProgramWindow(r);
}

}  // namespace tmbasic
