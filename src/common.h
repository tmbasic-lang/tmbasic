// this is the precompiled header
#ifndef COMMON_H_
#define COMMON_H_

#include <decimal.hh>

#define Uses_MsgBox
#define Uses_TApplication
#define Uses_TButton
#define Uses_TCheckBoxes
#define Uses_TDeskTop
#define Uses_TDialog
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
#define Uses_TScroller
#define Uses_TStaticText
#define Uses_TStatusDef
#define Uses_TStatusItem
#define Uses_TStatusLine
#define Uses_TStrListMaker
#define Uses_TStringCollection
#define Uses_TStringList
#define Uses_TSubMenu
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
#include <exception>
#include <fstream>
#include <initializer_list>
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

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef CLANG_TIDY
// clang-tidy gets upset about assert()
#undef assert
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define assert(expr) (void(expr))
#endif

// clang-tidy likes to see gsl::owner to express ownership of raw pointers, but we don't care about any other part of
// the C++ Guidelines Support Library. let's just define our own gsl::owner.
namespace gsl {
template <class T>
using owner = T;
}  // namespace gsl

#endif  // COMMON_H_
