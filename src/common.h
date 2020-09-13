// this is the precompiled header
#ifndef _COMMON_H_
#define _COMMON_H_

#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>
#include <cstdlib>
#include <cstdint>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#define Uses_MsgBox
#define Uses_TApplication
#define Uses_TDeskTop
#define Uses_TEvent
#define Uses_TKeys
#define Uses_TMenuBar
#define Uses_TMenuItem
#define Uses_TRect
#define Uses_TStatusDef
#define Uses_TStatusItem
#define Uses_TStatusLine
#define Uses_TSubMenu
#define Uses_TWindow
#include <tvision/tv.h>

#define IMMER_NO_THREAD_SAFETY 1
#define IMMER_NO_FREE_LIST 1
#include <immer/array.hpp>
#include <immer/array_transient.hpp>
#include <immer/map.hpp>
#include <immer/map_transient.hpp>
#include <immer/vector.hpp>
#include <immer/vector_transient.hpp>

#endif  // _COMMON_H_
