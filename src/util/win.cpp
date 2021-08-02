#include "win.h"

#ifdef _WIN32
namespace util {

std::wstring winUtf8ToUtf16(const std::string& utf8str) {
    auto utf16len = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, nullptr, 0);
    if (utf16len == 0) {
        return L"";
    }
    std::wstring utf16str;
    utf16str.resize(utf16len);
    MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, &utf16str[0], utf16len);
    return utf16str;
}

std::string winUtf16ToUtf8(const std::wstring& utf16str) {
    auto utf8len = WideCharToMultiByte(CP_UTF8, 0, utf16str.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (utf8len == 0) {
        return "";
    }
    std::string utf8str;
    utf8str.resize(utf8len);
    WideCharToMultiByte(CP_UTF8, 0, utf16str.c_str(), -1, &utf8str[0], utf8len, nullptr, nullptr);
    return utf8str;
}

struct WcharBufferLocalFree {
    void operator()(gsl::owner<wchar_t*> buf) {
        if (buf) {
            LocalFree(buf);
        }
    }
};

std::string winFormatErrorMessage(uint32_t winError) {
    gsl::owner<wchar_t*> messageBuffer = nullptr;
    auto messageLength = FormatMessageW(
        /* dwFlags */ FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        /* lpSource */ nullptr,
        /* dwMessageId */ winError,
        /* dwLanguageId */ MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        /* lpBuffer */ (LPWSTR)&messageBuffer,
        /* nSize */ 0,
        /* *Arguments */ nullptr);
    std::unique_ptr<wchar_t, WcharBufferLocalFree> messageBufferPtr{ messageBuffer };
    std::wstring wmessage(messageBuffer, messageLength);
    return winUtf16ToUtf8(wmessage);
}

}  // namespace util
#endif
