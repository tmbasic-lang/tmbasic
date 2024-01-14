#include "shared/console.h"

namespace shared {

void setUtf8Locale() {
#ifndef _WIN32
    auto* lang = getenv("LANG");
    if (lang == nullptr || std::string(lang).find("UTF-8") == std::string::npos) {
        setenv("LANG", "C.UTF-8", 1);
    }

    auto* lcAll = setlocale(LC_ALL, nullptr);
    if (lcAll == nullptr || std::string(lcAll).find("UTF-8") == std::string::npos) {
        setlocale(LC_ALL, "C.UTF-8");
    }
#endif
}

}  // namespace shared
