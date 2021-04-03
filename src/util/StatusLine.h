#pragma once

#include "../common.h"

namespace util {

class StatusItemColors {
   public:
    std::optional<TAttrPair> colorPairNormal;
    std::optional<TAttrPair> colorPairNormalDisabled;
};

class StatusLine : public TStatusLine {
   public:
    explicit StatusLine(const TRect& bounds, TStatusDef& aDefs);  // NOLINT(runtime/references)
    void draw() override;
    StatusItemColors* addStatusItemColors(TStatusItem* statusItem);
    static void setItemText(TStatusItem* statusItem, const std::string& text);

   private:
    std::unordered_map<ushort, std::unique_ptr<StatusItemColors>> _colors;  // by cmd
};

}  // namespace util
