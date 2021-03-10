#include "initConsole.h"

namespace util {

#ifdef _WIN32
class FontSearchData {
   public:
    bool hasConsolas = false;
    bool hasLucidaConsole = false;
};

static int CALLBACK fontFamilyCallback(const LOGFONT* lpelfe, const TEXTMETRIC* lpntme, DWORD FontType, LPARAM lParam) {
    auto* data = reinterpret_cast<FontSearchData*>(lParam);

    if (_tcscmp(lpelfe->lfFaceName, TEXT("Consolas")) == 0) {
        data->hasConsolas = true;
    } else if (_tcscmp(lpelfe->lfFaceName, TEXT("Lucida Console")) == 0) {
        data->hasLucidaConsole = true;
    }

    // https://docs.microsoft.com/en-us/previous-versions/dd162618(v=vs.85)
    // The return value must be a nonzero value to continue enumeration; to stop enumeration, the return value must be
    // zero.

    return 1;
}

static void initConsoleWin32() {
    // on failure, just return and hope for the best.

    // get a handle to the active screen buffer. the docs suggest that you should pass CONOUT$ to CreateFile, but every
    // example online uses GetStdHandle(STD_OUTPUT_HANDLE), which does indeed seem to work.

    auto activeScreenBuffer = GetStdHandle(STD_OUTPUT_HANDLE);

    // get the current console font
    //
    // https://docs.microsoft.com/en-us/windows/console/console-font-infoex
    // "cbSize: ... This member must be set to sizeof(CONSOLE_FONT_INFOEX) before calling GetCurrentConsoleFontEx or
    // it will fail."
    //
    // https://docs.microsoft.com/en-us/windows/console/getcurrentconsolefontex
    // "If the function fails, the return value is zero."

    CONSOLE_FONT_INFOEX fontInfo = { 0 };
    fontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    if (GetCurrentConsoleFontEx(
            /* hConsoleOutput */ activeScreenBuffer,
            /* bMaximumWindow */ FALSE,
            /* lpConsoleCurrentFontEx */ &fontInfo) == 0) {
        return;
    }

    // determine if this is a TrueType font. we don't need to do anything if so.
    //
    // https://docs.microsoft.com/en-us/windows/console/console-font-infoex
    // "FontFamily: The font pitch and family. For information about the possible values for this member, see the
    // description of the tmPitchAndFamily member of the TEXTMETRIC structure."
    //
    // https://docs.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-textmetricw
    // "tmPitchAndFamily: ... The four low-order bits of this member specify information about the pitch and the
    // technology of the font. A constant is defined for each of the four bits."
    // "TMPF_TRUETYPE: If this bit is set the font is a TrueType font."

    if ((fontInfo.FontFamily & TMPF_TRUETYPE) != 0) {
        return;
    }

    // this is a raster font, so let's try to find a TrueType font. either Consolas or Lucida Console should be
    // available.
    //
    // https://docs.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-enumfontfamiliesexw
    // "lpLogfont: ... The function examines the following members."
    //      "lfCharSet, lfFaceName, lfPitchAndFamily"
    // "dwFlags: This parameter is not used and must be zero."

    FontSearchData fontSearchData;
    LOGFONT logFont = { 0 };
    logFont.lfCharSet = DEFAULT_CHARSET;
    EnumFontFamiliesEx(
        /* hdc */ GetDC(0),
        /* lpLogfont */ &logFont,
        /* lpProc */ fontFamilyCallback,
        /* lParam */ reinterpret_cast<LPARAM>(&fontSearchData),
        /* dwFlags */ 0);

    LPCTSTR fontName = nullptr;
    if (fontSearchData.hasConsolas) {
        fontName = TEXT("Consolas");
    } else if (fontSearchData.hasLucidaConsole) {
        fontName = TEXT("Lucida Console");
    } else {
        return;  // no good fonts available
    }

    // set the console font to our pick.

    fontInfo = { 0 };
    fontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);
    fontInfo.FontFamily = FF_DONTCARE;
    fontInfo.FontWeight = FW_NORMAL;
    fontInfo.dwFontSize.X = 0;
    fontInfo.dwFontSize.Y = 24;

#ifdef _UNICODE
    wcscpy_s(
        /* dest */ fontInfo.FaceName,
        /* destsz */ LF_FACESIZE,
        /* src */ fontName);
#else
    // FaceName is a wide string even on ANSI builds
    mbstowcs(
        /* dst */ fontInfo.FaceName,
        /* src */ fontName,
        /* len */ LF_FACESIZE);
#endif

    // https://docs.microsoft.com/en-us/windows/console/setcurrentconsolefontex
    // "If the function fails, the return value is zero. To get extended error information, call GetLastError."

    if (SetCurrentConsoleFontEx(
            /* hConsoleOutput */ activeScreenBuffer,
            /* bMaximumWindow */ FALSE,
            /* lpConsoleCurrentFontEx */ &fontInfo) == 0) {
        return;
    }
}
#endif

void initConsole() {
#ifdef _WIN32
    initConsoleWin32();
#endif
}

}  // namespace util
