#include "tmbasic/ProgramWindow.h"
#include "../../obj/helpfile.h"
#include "shared/util/path.h"
#include "shared/vm/Program.h"
#include "tmbasic/constants.h"
#include "tmbasic/tvutil.h"

using vm::Program;

namespace tmbasic {

enum class ProgramItemType { kProcedure, kGlobal, kConstant, kType };

static const char* kProgramItemTypeStrings[] = {
    "Procedures \x10",
    "Globals    \x10",
    "Constants  \x10",
    "Types      \x10",
};

typedef std::function<void(ProgramItemType)> ProgramItemTypeSelectedFunc;

class ProgramItemTypesListBox : public TListViewer {
   public:
    ProgramItemTypesListBox(const TRect& bounds, ushort numCols, ProgramItemTypeSelectedFunc onSelectedFunc)
        : TListViewer(bounds, numCols, nullptr, nullptr), _onSelectedFunc(onSelectedFunc) {
        setRange(4);
    }

    virtual ~ProgramItemTypesListBox() {}

    void getText(char* dest, int16_t item, int16_t maxLen) override {
        strncpy(dest, kProgramItemTypeStrings[item], maxLen);
        dest[maxLen] = '\0';
    }

    void focusItem(int16_t item) override {
        TListViewer::focusItem(item);
        _onSelectedFunc(static_cast<ProgramItemType>(item));
    }

    TPalette& getPalette() const override {
        // Active, Inactive, Focused, Selected, Divider
        static const char bytes[] = kWindowPaletteFrameActive kWindowPaletteFramePassive
            kWindowPaletteScrollerSelectedText kWindowPaletteScrollBarControls kWindowPaletteFrameActive;
        static auto palette = TPalette(bytes, sizeof(bytes) - 1);
        return palette;
    }

   private:
    ProgramItemTypeSelectedFunc _onSelectedFunc;
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
        curCommandSet.enableCmd(cmSave);
        curCommandSet.enableCmd(cmSaveAs);
    }

    virtual ~ProgramContentsListBox() {}

    void setData(std::vector<ProgramItem> items) {
        _items = std::move(items);
        setRange(_items.size());
        drawView();
    }

    void getText(char* dest, int16_t item, int16_t maxLen) override {
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

ProgramWindow::ProgramWindow(const TRect& r, std::optional<std::string> filePath)
    : TWindow(r, "Untitled (program)", wnNoNumber),
      TWindowInit(TWindow::initFrame),
      _vmProgram(std::make_unique<Program>()),
      _sourceProgram(std::make_unique<SourceProgram>()),
      _dirty(!filePath.has_value()) {
    palette = 0;
    auto* vScrollBar = new TScrollBar(TRect(size.x - 1, 1, size.x, size.y - 1));
    insert(vScrollBar);

    auto typesListBoxRect = getExtent();
    typesListBoxRect.grow(-1, -1);
    typesListBoxRect.b.x = 15;
    _typesListBox = new ProgramItemTypesListBox(typesListBoxRect, 1, [](ProgramItemType programItemType) {
        if (programItemType == ProgramItemType::kType) {
            // exit(-1);
        }
    });
    _typesListBox->growMode = gfGrowHiY;
    _typesListBox->options |= ofFramed;
    insert(_typesListBox);

    auto contentsListBoxRect = getExtent();
    contentsListBoxRect.grow(-1, -1);
    contentsListBoxRect.a.x = 16;
    _contentsListBox = new ProgramContentsListBox(contentsListBoxRect, 1, vScrollBar);
    _contentsListBox->growMode = gfGrowHiX | gfGrowHiY;
    insert(_contentsListBox);

    auto v = std::vector<ProgramItem>();
    _contentsListBox->setData(std::move(v));

    // TODO: open file
    _filePath = filePath;
    updateTitle();
}

ProgramWindow::~ProgramWindow() {}

TPalette& ProgramWindow::getPalette() const {
    static auto palette = TPalette(cpBlueWindow, sizeof(cpBlueWindow) - 1);
    return palette;
}

ushort ProgramWindow::getHelpCtx() {
    return hcide_programWindow;
}

void ProgramWindow::handleEvent(TEvent& event) {
    TWindow::handleEvent(event);
    if (event.what == evBroadcast) {
        switch (event.message.command) {
            case kCmdProgramSave:
                onSave();
                break;

            case kCmdProgramSaveAs:
                onSaveAs();
                break;

            case kCmdAppExit:
                if (!preClose()) {
                    *static_cast<bool*>(event.message.infoPtr) = true;
                    clearEvent(event);
                }
                break;

            case kCmdFindProgramWindow:
                *static_cast<ProgramWindow**>(event.message.infoPtr) = this;
                clearEvent(event);
                break;
        }
    }
}

bool ProgramWindow::onSave() {
    if (_filePath.has_value()) {
        return save(*_filePath);
    } else {
        return onSaveAs();
    }
}

bool ProgramWindow::onSaveAs() {
    auto* d = new TFileDialog("*.bas", "Save Program As (.BAS)", "~N~ame", fdOKButton, 101);
    auto result = false;

    if (TProgram::deskTop->execView(d) != cmCancel) {
        char fileName[MAXPATH];
        d->getFileName(fileName);
        if (save(fileName)) {
            _filePath = fileName;
            updateTitle();
            frame->drawView();
            result = true;
        }
    }
    destroy(d);

    return result;
}

bool ProgramWindow::save(std::string filePath) {
    try {
        _sourceProgram->save(filePath);
        _filePath = filePath;
        return true;
    } catch (const std::system_error& ex) {
        std::ostringstream s;
        s << "Save failed: " << ex.what();
        messageBox(s.str(), mfInformation | mfOKButton);
        return false;
    } catch (...) {
        messageBox("Save failed.", mfInformation | mfOKButton);
        return false;
    }
}

void ProgramWindow::updateTitle() {
    std::ostringstream s;
    if (_dirty) {
        s << kCharStar;
    }
    if (_filePath.has_value()) {
        s << util::getFileName(*_filePath);
    } else {
        s << "Untitled";
    }
    s << " (program)";

    delete[] title;
    title = strdup(s.str().c_str());
}

// true = close, false = stay open
bool ProgramWindow::preClose() {
    if (_dirty) {
        std::ostringstream s;
        s << "Save changes to \"" << (_filePath.has_value() ? util::getFileName(*_filePath) : "Untitled") << "\"?";
        auto result = messageBox(s.str(), mfWarning | mfYesNoCancel);
        if (result == cmCancel) {
            return false;
        } else if (result == cmYes) {
            if (!onSave()) {
                return false;
            }
        }
    }

    return true;
}

void ProgramWindow::close() {
    if (preClose()) {
        // close all other program-related windows first
        message(owner, evBroadcast, kCmdCloseProgramRelatedWindows, nullptr);

        TWindow::close();
    }
}

void ProgramWindow::enableDisableMenuCommands() {
    enableDisableCommand(cmSave, _dirty);
    enableDisableCommand(cmSaveAs, true);
}

bool ProgramWindow::isDirty() {
    return _dirty;
}

void ProgramWindow::addNewSourceMember(std::unique_ptr<SourceMember> sourceMember) {
    _sourceProgram->members.push_back(std::move(sourceMember));
}

}  // namespace tmbasic
