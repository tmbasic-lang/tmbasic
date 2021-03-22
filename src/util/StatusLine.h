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
    explicit StatusLine(const TRect& bounds, TStatusDef& aDefs);
    void draw() override;
    StatusItemColors* addStatusItemColors(TStatusItem* statusItem);

   private:
    std::unordered_map<ushort, std::unique_ptr<StatusItemColors>> _colors;  // by cmd
};

}  // namespace util
