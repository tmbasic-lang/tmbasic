// this is the precompiled header
#ifndef _COMMON_H_
#define _COMMON_H_

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <decimal/decimal>
#include <fstream>
#include <initializer_list>
#include <ios>
#include <iostream>
#include <memory>
#include <optional>
#include <regex>
#include <sstream>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// don't use tvision/borland/fstream.h which tv.h will try to include
#define FSTREAM_H

#define Uses_MsgBox
#define Uses_TApplication
#define Uses_TDeskTop
#define Uses_TDialog
#define Uses_TEditor
#define Uses_TEvent
#define Uses_TIndicator
#define Uses_TKeys
#define Uses_TLabel
#define Uses_TMenuBar
#define Uses_TMenuItem
#define Uses_TOutlineViewer
#define Uses_TRect
#define Uses_TStaticText
#define Uses_TStatusDef
#define Uses_TStatusItem
#define Uses_TStatusLine
#define Uses_TSubMenu
#define Uses_TWindow
#define Uses_fpstream
#include <tvision/tv.h>
#include <tvision/help.h>

#define IMMER_NO_THREAD_SAFETY 1
#define IMMER_NO_FREE_LIST 1
#include <immer/array.hpp>
#include <immer/array_transient.hpp>
#include <immer/map.hpp>
#include <immer/map_transient.hpp>
#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>

#endif  // _COMMON_H_
