#include "InsertColorDialog.h"
#include "../../obj/resources/help/helpfile.h"
#include "../util/Button.h"
#include "../util/InputLine.h"
#include "../util/Label.h"
#include "../util/ViewPtr.h"
#include "../util/tvutil.h"
#include "GridLayout.h"
#include "events.h"

using util::Button;
using util::InputLine;
using util::Label;
using util::parseUserInt;
using util::ViewPtr;

namespace tmbasic {

static TColorDesired makeColor(int red, int green, int blue) {
    int rgb = red;
    rgb <<= 8;
    rgb |= green;
    rgb <<= 8;
    rgb |= blue;
    return { rgb };
}

static std::array<TColorDesired, 256> _xtermColors = {
    makeColor(0, 0, 0),       makeColor(128, 0, 0),     makeColor(0, 128, 0),     makeColor(128, 128, 0),
    makeColor(0, 0, 128),     makeColor(128, 0, 128),   makeColor(0, 128, 128),   makeColor(192, 192, 192),
    makeColor(128, 128, 128), makeColor(255, 0, 0),     makeColor(0, 255, 0),     makeColor(255, 255, 0),
    makeColor(0, 0, 255),     makeColor(255, 0, 255),   makeColor(0, 255, 255),   makeColor(255, 255, 255),
    makeColor(0, 0, 0),       makeColor(0, 0, 95),      makeColor(0, 0, 135),     makeColor(0, 0, 175),
    makeColor(0, 0, 215),     makeColor(0, 0, 255),     makeColor(0, 95, 0),      makeColor(0, 95, 95),
    makeColor(0, 95, 135),    makeColor(0, 95, 175),    makeColor(0, 95, 215),    makeColor(0, 95, 255),
    makeColor(0, 135, 0),     makeColor(0, 135, 95),    makeColor(0, 135, 135),   makeColor(0, 135, 175),
    makeColor(0, 135, 215),   makeColor(0, 135, 255),   makeColor(0, 175, 0),     makeColor(0, 175, 95),
    makeColor(0, 175, 135),   makeColor(0, 175, 175),   makeColor(0, 175, 215),   makeColor(0, 175, 255),
    makeColor(0, 215, 0),     makeColor(0, 215, 95),    makeColor(0, 215, 135),   makeColor(0, 215, 175),
    makeColor(0, 215, 215),   makeColor(0, 215, 255),   makeColor(0, 255, 0),     makeColor(0, 255, 95),
    makeColor(0, 255, 135),   makeColor(0, 255, 175),   makeColor(0, 255, 215),   makeColor(0, 255, 255),
    makeColor(95, 0, 0),      makeColor(95, 0, 95),     makeColor(95, 0, 135),    makeColor(95, 0, 175),
    makeColor(95, 0, 215),    makeColor(95, 0, 255),    makeColor(95, 95, 0),     makeColor(95, 95, 95),
    makeColor(95, 95, 135),   makeColor(95, 95, 175),   makeColor(95, 95, 215),   makeColor(95, 95, 255),
    makeColor(95, 135, 0),    makeColor(95, 135, 95),   makeColor(95, 135, 135),  makeColor(95, 135, 175),
    makeColor(95, 135, 215),  makeColor(95, 135, 255),  makeColor(95, 175, 0),    makeColor(95, 175, 95),
    makeColor(95, 175, 135),  makeColor(95, 175, 175),  makeColor(95, 175, 215),  makeColor(95, 175, 255),
    makeColor(95, 215, 0),    makeColor(95, 215, 95),   makeColor(95, 215, 135),  makeColor(95, 215, 175),
    makeColor(95, 215, 215),  makeColor(95, 215, 255),  makeColor(95, 255, 0),    makeColor(95, 255, 95),
    makeColor(95, 255, 135),  makeColor(95, 255, 175),  makeColor(95, 255, 215),  makeColor(95, 255, 255),
    makeColor(135, 0, 0),     makeColor(135, 0, 95),    makeColor(135, 0, 135),   makeColor(135, 0, 175),
    makeColor(135, 0, 215),   makeColor(135, 0, 255),   makeColor(135, 95, 0),    makeColor(135, 95, 95),
    makeColor(135, 95, 135),  makeColor(135, 95, 175),  makeColor(135, 95, 215),  makeColor(135, 95, 255),
    makeColor(135, 135, 0),   makeColor(135, 135, 95),  makeColor(135, 135, 135), makeColor(135, 135, 175),
    makeColor(135, 135, 215), makeColor(135, 135, 255), makeColor(135, 175, 0),   makeColor(135, 175, 95),
    makeColor(135, 175, 135), makeColor(135, 175, 175), makeColor(135, 175, 215), makeColor(135, 175, 255),
    makeColor(135, 215, 0),   makeColor(135, 215, 95),  makeColor(135, 215, 135), makeColor(135, 215, 175),
    makeColor(135, 215, 215), makeColor(135, 215, 255), makeColor(135, 255, 0),   makeColor(135, 255, 95),
    makeColor(135, 255, 135), makeColor(135, 255, 175), makeColor(135, 255, 215), makeColor(135, 255, 255),
    makeColor(175, 0, 0),     makeColor(175, 0, 95),    makeColor(175, 0, 135),   makeColor(175, 0, 175),
    makeColor(175, 0, 215),   makeColor(175, 0, 255),   makeColor(175, 95, 0),    makeColor(175, 95, 95),
    makeColor(175, 95, 135),  makeColor(175, 95, 175),  makeColor(175, 95, 215),  makeColor(175, 95, 255),
    makeColor(175, 135, 0),   makeColor(175, 135, 95),  makeColor(175, 135, 135), makeColor(175, 135, 175),
    makeColor(175, 135, 215), makeColor(175, 135, 255), makeColor(175, 175, 0),   makeColor(175, 175, 95),
    makeColor(175, 175, 135), makeColor(175, 175, 175), makeColor(175, 175, 215), makeColor(175, 175, 255),
    makeColor(175, 215, 0),   makeColor(175, 215, 95),  makeColor(175, 215, 135), makeColor(175, 215, 175),
    makeColor(175, 215, 215), makeColor(175, 215, 255), makeColor(175, 255, 0),   makeColor(175, 255, 95),
    makeColor(175, 255, 135), makeColor(175, 255, 175), makeColor(175, 255, 215), makeColor(175, 255, 255),
    makeColor(215, 0, 0),     makeColor(215, 0, 95),    makeColor(215, 0, 135),   makeColor(215, 0, 175),
    makeColor(215, 0, 215),   makeColor(215, 0, 255),   makeColor(215, 95, 0),    makeColor(215, 95, 95),
    makeColor(215, 95, 135),  makeColor(215, 95, 175),  makeColor(215, 95, 215),  makeColor(215, 95, 255),
    makeColor(215, 135, 0),   makeColor(215, 135, 95),  makeColor(215, 135, 135), makeColor(215, 135, 175),
    makeColor(215, 135, 215), makeColor(215, 135, 255), makeColor(215, 175, 0),   makeColor(215, 175, 95),
    makeColor(215, 175, 135), makeColor(215, 175, 175), makeColor(215, 175, 215), makeColor(215, 175, 255),
    makeColor(215, 215, 0),   makeColor(215, 215, 95),  makeColor(215, 215, 135), makeColor(215, 215, 175),
    makeColor(215, 215, 215), makeColor(215, 215, 255), makeColor(215, 255, 0),   makeColor(215, 255, 95),
    makeColor(215, 255, 135), makeColor(215, 255, 175), makeColor(215, 255, 215), makeColor(215, 255, 255),
    makeColor(255, 0, 0),     makeColor(255, 0, 95),    makeColor(255, 0, 135),   makeColor(255, 0, 175),
    makeColor(255, 0, 215),   makeColor(255, 0, 255),   makeColor(255, 95, 0),    makeColor(255, 95, 95),
    makeColor(255, 95, 135),  makeColor(255, 95, 175),  makeColor(255, 95, 215),  makeColor(255, 95, 255),
    makeColor(255, 135, 0),   makeColor(255, 135, 95),  makeColor(255, 135, 135), makeColor(255, 135, 175),
    makeColor(255, 135, 215), makeColor(255, 135, 255), makeColor(255, 175, 0),   makeColor(255, 175, 95),
    makeColor(255, 175, 135), makeColor(255, 175, 175), makeColor(255, 175, 215), makeColor(255, 175, 255),
    makeColor(255, 215, 0),   makeColor(255, 215, 95),  makeColor(255, 215, 135), makeColor(255, 215, 175),
    makeColor(255, 215, 215), makeColor(255, 215, 255), makeColor(255, 255, 0),   makeColor(255, 255, 95),
    makeColor(255, 255, 135), makeColor(255, 255, 175), makeColor(255, 255, 215), makeColor(255, 255, 255),
    makeColor(8, 8, 8),       makeColor(18, 18, 18),    makeColor(28, 28, 28),    makeColor(38, 38, 38),
    makeColor(48, 48, 48),    makeColor(58, 58, 58),    makeColor(68, 68, 68),    makeColor(78, 78, 78),
    makeColor(88, 88, 88),    makeColor(98, 98, 98),    makeColor(108, 108, 108), makeColor(118, 118, 118),
    makeColor(128, 128, 128), makeColor(138, 138, 138), makeColor(148, 148, 148), makeColor(158, 158, 158),
    makeColor(168, 168, 168), makeColor(178, 178, 178), makeColor(188, 188, 188), makeColor(198, 198, 198),
    makeColor(208, 208, 208), makeColor(218, 218, 218), makeColor(228, 228, 228), makeColor(238, 238, 238),
};

class ColorView : public TView {
   public:
    int red = 255;
    int green = 255;
    int blue = 255;
    explicit ColorView(const TRect& bounds) : TView(bounds) {}
    TColorAttr mapColor(uchar /*index*/) override {
        auto color = makeColor(red, green, blue);
        return TColorAttr(color, color);
    }
};

class PaletteView : public TView {
   public:
    std::function<void(TColorDesired)> onSetColor;

    explicit PaletteView(const TRect& bounds) : TView(bounds) {
        options |= ofFramed;
        for (auto i = 0; i < 16; i++) {
            auto color = _xtermColors.at(i);
            for (auto j = 0; j < 5; j++) {
                _colors.at(i / 8).at(1 + (i % 8) * 5 + j) = color;
            }
        }
        _colors.at(0).at(0) = _xtermColors.at(0);
        _colors.at(0).at(41) = _xtermColors.at(7);
        _colors.at(1).at(0) = _xtermColors.at(8);
        _colors.at(1).at(41) = _xtermColors.at(15);

        auto x = 0;
        auto y = 2;
        auto col = -1;
        for (auto i = 16; i < 256; i++) {
            if (((i - 16) % 36) == 0) {
                col++;
                y = 2;
                x = col * 6;
            } else if (((i - 16) % 6) == 0) {
                y++;
                x = col * 6;
            } else {
                x++;
            }
            _colors.at(y).at(x) = _xtermColors.at(i);
        }

        for (auto x = 36; x < 42; x++) {
            for (auto y = 6; y < 8; y++) {
                _colors.at(y).at(x) = _xtermColors.at(15);
            }
        }
    }

    void draw() override {
        for (size_t i = 0; i < _colors.size(); i++) {
            const std::array<TColorDesired, 42>& row = _colors.at(i);
            TDrawBuffer b;
            for (size_t j = 0; j < row.size(); j++) {
                auto color = row.at(j);
                b.moveChar(j, ' ', TColorAttr(color, color), 1);
            }
            writeLine(0, i, row.size(), 1, b);
        }
    }

    void handleEvent(TEvent& event) override {
        if (event.what == evMouseDown) {
            do {
                if (mouseInView(event.mouse.where)) {
                    TPoint spot = makeLocal(event.mouse.where);
                    auto x = static_cast<size_t>(spot.x);
                    auto y = static_cast<size_t>(spot.y);
                    if (y < _colors.size() && x < _colors.at(y).size()) {
                        onSetColor(_colors.at(y).at(x));
                        if ((event.mouse.eventFlags & meDoubleClick) != 0) {
                            TEvent okEvent = { 0 };
                            okEvent.what = evCommand;
                            okEvent.message.command = cmOK;
                            putEvent(okEvent);
                            clearEvent(event);
                            return;
                        }
                    }
                }
            } while (mouseEvent(event, evMouseMove));
            clearEvent(event);
        }
    }

   private:
    std::array<std::array<TColorDesired, 42>, 8> _colors{};
};

class InsertColorDialogPrivate {
   public:
    ViewPtr<PaletteView> paletteView{ TRect(3, 2, 45, 10) };
    ViewPtr<ColorView> colorView{ TRect(0, 0, 6, 2) };
    ViewPtr<InputLine> redInputLine{ 255, 6, 3 };
    ViewPtr<InputLine> greenInputLine{ 255, 6, 3 };
    ViewPtr<InputLine> blueInputLine{ 255, 6, 3 };
    ViewPtr<Button> insertButton{ "Insert", cmOK, bfDefault };
};

InsertColorDialog::InsertColorDialog(const std::string& title, const std::string& insertButtonText)
    : TDialog(TRect(0, 0, 67, 15), title), TWindowInit(&TDialog::initFrame), _private(new InsertColorDialogPrivate()) {
    options |= ofCentered;

    _private->insertButton->setTitle(insertButtonText);

    _private->paletteView->onSetColor = [this](TColorDesired color) -> void {
        auto rgb = color.asRGB();
        auto red = std::to_string(rgb.r);
        auto green = std::to_string(rgb.g);
        auto blue = std::to_string(rgb.b);
        _private->colorView->red = rgb.r;
        _private->colorView->green = rgb.g;
        _private->colorView->blue = rgb.b;
        strcpy(_private->redInputLine->data, red.c_str());
        strcpy(_private->greenInputLine->data, green.c_str());
        strcpy(_private->blueInputLine->data, blue.c_str());
        _private->insertButton->focus();
        _private->redInputLine->drawView();
        _private->greenInputLine->drawView();
        _private->blueInputLine->drawView();
        _private->colorView->drawView();
        selection = color;
    };

    _private->paletteView.addTo(this);
    _private->colorView->options |= ofFramed;

    GridLayout(
        2,
        {
            new Label("Red:"),
            _private->redInputLine.take(),
            new Label("Green:"),
            _private->greenInputLine.take(),
            new Label("Blue:"),
            _private->blueInputLine.take(),
            nullptr,
            _private->colorView.take(),
        })
        .apply(this, TPoint{ 45, 0 });

    RowLayout(
        true,
        {
            _private->insertButton.take(),
            new Button("Cancel", cmCancel, bfNormal),
        })
        .addTo(this, 0, 64, 12);

    _private->redInputLine->focus();
}

InsertColorDialog::~InsertColorDialog() {
    delete _private;
}

void InsertColorDialog::handleEvent(TEvent& event) {
    if (event.what == evCommand && event.message.command == cmOK) {
        try {
            auto red = parseUserInt(_private->redInputLine->data, "red component", 0, 255);
            auto green = parseUserInt(_private->greenInputLine->data, "green component", 0, 255);
            auto blue = parseUserInt(_private->blueInputLine->data, "blue component", 0, 255);
            selection = makeColor(red, green, blue);
        } catch (std::runtime_error& ex) {
            messageBox(ex.what(), mfError | mfOKButton);
            clearEvent(event);
        }
    } else if (event.what == evBroadcast && event.message.command == kCmdTimerTick) {
        try {
            auto red = parseUserInt(_private->redInputLine->data, "", 0, 255);
            auto green = parseUserInt(_private->greenInputLine->data, "", 0, 255);
            auto blue = parseUserInt(_private->blueInputLine->data, "", 0, 255);
            selection = makeColor(red, green, blue);
            if (_private->colorView->red != red || _private->colorView->green != green ||
                _private->colorView->blue != blue) {
                _private->colorView->red = red;
                _private->colorView->green = green;
                _private->colorView->blue = blue;
                _private->colorView->drawView();
            }
        } catch (std::runtime_error& e) {
            // ignore
        }
    }

    TDialog::handleEvent(event);
}

}  // namespace tmbasic
