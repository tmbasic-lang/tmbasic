#pragma once

#include "../common.h"

namespace util {

#ifdef _WIN32
std::wstring winUtf8ToUtf16(const std::string& utf8str);
std::string winUtf16ToUtf8(const std::wstring& utf16str);
std::string winFormatErrorMessage(uint32_t winError);
#endif

}  // namespace util
