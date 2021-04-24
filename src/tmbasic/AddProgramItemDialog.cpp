#include "AddProgramItemDialog.h"
#include "../util/Button.h"
#include "../util/Label.h"
#include "../util/ListViewer.h"
#include "../util/ViewPtr.h"
#include "GridLayout.h"
#include "RowLayout.h"
#include "events.h"

using util::Button;
using util::Label;
using util::ListViewer;
using util::ViewPtr;

namespace tmbasic {

static std::vector<std::string> kAddProgramItemStrings{
    "F1 Subroutine", "F2 Function", "F3 Global variable", "F4 Constant",
    "F5 Type",       "F6 Form",     "F7 Custom control",  "F8 Picture",
};

static std::unordered_map<uint16_t, uint16_t> kAddProgramItemCommands{
    { kbF1, kCmdProgramAddSubroutine },    { kbF2, kCmdProgramAddFunction }, { kbF3, kCmdProgramAddGlobalVariable },
    { kbF4, kCmdProgramAddConstant },      { kbF5, kCmdProgramAddType },     { kbF6, kCmdProgramAddForm },
    { kbF7, kCmdProgramAddCustomControl }, { kbF8, kCmdProgramAddPicture },
};

static std::vector<uint16_t> kAddProgramItemKeys{
    kbF1, kbF2, kbF3, kbF4, kbF5, kbF6, kbF7, kbF8,
};

class AddProgramItemListBox : public ListViewer {
   public:
    AddProgramItemListBox(const TRect& bounds, uint16_t numCols) : ListViewer(bounds, numCols, nullptr, nullptr) {
        setRange(kAddProgramItemStrings.size());
    }

    void getText(char* dest, int16_t item, int16_t maxLen) override {
        strncpy(dest, kAddProgramItemStrings[item].c_str(), maxLen);
        dest[maxLen] = '\0';  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }

    void selectItem(int16_t item) override {
        message(owner, evCommand, cmOK, nullptr);
        ListViewer::selectItem(item);
    }
};

class AddProgramItemDialogPrivate {
   public:
    ViewPtr<AddProgramItemListBox> listbox{ TRect{ 0, 0, 45, 4 }, 2 };
};

AddProgramItemDialog::AddProgramItemDialog()
    : TDialog(TRect{ 0, 0, 0, 0 }, "Add New Item"),
      TWindowInit(&TDialog::initFrame),
      _private(new AddProgramItemDialogPrivate()) {
    options |= ofCentered;
    GridLayout(
        1,
        {
            new Label("Choose an item type."),
            RowLayout(
                false,
                {
                    _private->listbox.take(),
                })
                .setMarginX(1),
            RowLayout(
                true,
                {
                    new Button("Add", cmOK, bfDefault),
                    new Button("Cancel", cmCancel, bfNormal),
                }),
        })
        .addTo(this);
    _private->listbox->focus();
}

AddProgramItemDialog::~AddProgramItemDialog() {
    delete _private;
}

void AddProgramItemDialog::handleEvent(TEvent& event) {
    if (event.what == evKeyDown) {
        auto cmd = kAddProgramItemCommands.find(event.keyDown.keyCode);
        if (cmd != kAddProgramItemCommands.end()) {
            clearEvent(event);
            endModal(cmd->second);
        }
    } else if (event.what == evCommand && event.message.command == cmOK) {
        auto index = _private->listbox->focused;
        auto key = kAddProgramItemKeys.at(index);
        auto cmd = kAddProgramItemCommands.find(key);
        clearEvent(event);
        endModal(cmd->second);
    }

    TDialog::handleEvent(event);
}

}  // namespace tmbasic
