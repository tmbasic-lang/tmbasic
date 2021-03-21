#include "../util/Label.h"
#include "../util/tvutil.h"

namespace util {

Label::Label(TStringView text, TView* link) : TLabel(TRect(0, 0, static_cast<int>(cstrlen(text) + 3), 1), text, link) {}

}  // namespace util
