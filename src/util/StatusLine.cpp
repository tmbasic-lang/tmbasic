#include "StatusLine.h"

namespace util {

StatusLine::StatusLine(const TRect& bounds, TStatusDef& aDefs) : TStatusLine(bounds, aDefs) {}

void StatusLine::draw() {
    TDrawBuffer b;
    TAttrPair color{};

    TAttrPair cNormal = getColor(0x0301);
    TAttrPair cNormDisabled = getColor(0x0202);
    b.moveChar(0, ' ', cNormal, size.x);
    auto* t = items;
    ushort i = 0;

    while (t != nullptr) {
        StatusItemColors* colors = nullptr;
        auto foundColors = _colors.find(t->command);
        if (foundColors != _colors.end()) {
            colors = foundColors->second.get();
        }

        if (t->text != nullptr) {
            ushort l = cstrlen(t->text);
            if (i + l < size.x) {
                if (commandEnabled(t->command)) {
                    if (colors != nullptr && colors->colorPairNormal.has_value()) {
                        color = *colors->colorPairNormal;
                    } else {
                        color = cNormal;
                    }
                } else {
                    if (colors != nullptr && colors->colorPairNormalDisabled.has_value()) {
                        color = *colors->colorPairNormalDisabled;
                    } else {
                        color = cNormDisabled;
                    }
                }

                b.moveChar(i, ' ', color, 1);
                b.moveCStr(i + 1, t->text, color);
                b.moveChar(i + l + 1, ' ', color, 1);
            }
            i += l + 2;
        }
        t = t->next;
    }
    if (i < size.x - 2) {
        TStringView hintText = hint(helpCtx);
        if (!hintText.empty()) {
            b.moveStr(i, "\xB3 ", cNormal);
            i += 2;
            b.moveStr(i, hintText, cNormal, size.x - i);
        }
    }
    writeLine(0, 0, static_cast<int16_t>(size.x), 1, b);
}

StatusItemColors* StatusLine::addStatusItemColors(TStatusItem* statusItem) {
    auto x = std::make_unique<StatusItemColors>();
    auto* ptr = x.get();
    _colors[statusItem->command] = std::move(x);
    return ptr;
}

void StatusLine::setItemText(TStatusItem* statusItem, const std::string& text) {
    delete[] statusItem->text;
    if (text != statusItem->text) {
        statusItem->text = strdup(text.c_str());
    }
}

}  // namespace util
