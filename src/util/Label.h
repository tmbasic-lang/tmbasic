#pragma once

#include "../common.h"

namespace util {

class Label : public TLabel {
   public:
    explicit Label(const TRect& r);
    explicit Label(TStringView text, TView* link = nullptr);
    void setTitle(const std::string& title);
};

}  // namespace util
