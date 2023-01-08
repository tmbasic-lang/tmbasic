// this is the precompiled header
#ifndef COMMON_H_
#define COMMON_H_

#define U_HIDE_DEPRECATED_API 1
#include <unicode/brkiter.h>
#include <unicode/calendar.h>
#include <unicode/coll.h>
#include <unicode/simpletz.h>
#include <unicode/timezone.h>
#include <unicode/utypes.h>

#include <decimal.hh>

#define Uses_MsgBox
#define Uses_TApplication
#define Uses_TBackground
#define Uses_TButton
#define Uses_TCheckBoxes
#define Uses_TClipboard
#define Uses_TDeskTop
#define Uses_TDialog
#define Uses_TDrawBuffer
#define Uses_TEditor
#define Uses_TEvent
#define Uses_TFileDialog
#define Uses_TFrame
#define Uses_TIndicator
#define Uses_TInputLine
#define Uses_TKeys
#define Uses_TLabel
#define Uses_TListBox
#define Uses_TListViewer
#define Uses_TMenuBar
#define Uses_TMenuItem
#define Uses_TOutlineViewer
#define Uses_TPalette
#define Uses_TRect
#define Uses_TSItem
#define Uses_TScreen
#define Uses_TScroller
#define Uses_TStaticText
#define Uses_TStatusDef
#define Uses_TStatusItem
#define Uses_TStatusLine
#define Uses_TStrListMaker
#define Uses_TStringCollection
#define Uses_TStringList
#define Uses_TSubMenu
#define Uses_TText
#define Uses_TWindow
#define Uses_fpstream
#include <tvision/tv.h>
#include <tvision/help.h>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ctime>

#include <algorithm>
#include <array>
#include <chrono>
#include <codecvt>
#include <exception>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iomanip>
#include <ios>
#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <ratio>
#include <regex>
#include <sstream>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>

#define IMMER_NO_FREE_LIST 1      // NOLINT
#define IMMER_NO_THREAD_SAFETY 1  // NOLINT
#include <immer/array.hpp>
#include <immer/array_transient.hpp>
#include <immer/map.hpp>
#include <immer/map_transient.hpp>
#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>

#include <nameof.hpp>

#include <fmt/format.h>

#include <turbo/turbo.h>
#include <turbo/scintilla/include/SciLexer.h>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#endif

#ifdef CLANG_TIDY
// clang-tidy gets upset about assert()
#undef assert
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define assert(expr)                     \
    if (!(expr)) {                       \
        throw std::runtime_error(#expr); \
    }
#endif

// clang-tidy likes to see gsl::owner to express ownership of raw pointers, but we don't care about any other part of
// the C++ Guidelines Support Library. let's just define our own gsl::owner.
namespace gsl {
template <class T>
using owner = T;
}  // namespace gsl

#ifdef NDEBUG
#define debugLog(x)
#else
inline void debugLog(std::function<void(std::ofstream* stream)> x) {
    auto fileStream = std::ofstream("debug.log", std::ios::out | std::ios::app);
    x(&fileStream);
    fileStream << "\n";
    fileStream.close();
}
#endif

// ieee754.h not available everywhere
union ieee754_double {
    double d;
    struct {
        unsigned int mantissa1 : 32;
        unsigned int mantissa0 : 20;
        unsigned int exponent : 11;
        unsigned int negative : 1;
    } ieee;
};

#define IEEE754_DOUBLE_BIAS 0x3ff

#endif  // COMMON_H_
