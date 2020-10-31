#include "HelpResource.h"

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <mach-o/getsect.h>
#include <mach-o/ldsyms.h>
#endif

#ifdef __linux__
extern char _binary_help_h32_start[];
extern char _binary_help_h32_end[];
#endif

#ifdef _WIN32
extern char binary_help_h32_start[];
extern char binary_help_h32_end[];
#endif

namespace ui {

#ifdef __linux__
HelpResource::HelpResource()
    : start(reinterpret_cast<uint8_t*>(_binary_help_h32_start)),
      end(reinterpret_cast<uint8_t*>(_binary_help_h32_end)) {}
#endif

#ifdef _WIN32
HelpResource::HelpResource()
    : start(reinterpret_cast<uint8_t*>(binary_help_h32_start)), end(reinterpret_cast<uint8_t*>(binary_help_h32_end)) {}
#endif

#ifdef __APPLE__
HelpResource::HelpResource() {
    unsigned long size = 0;
    auto data = getsectiondata(&_mh_execute_header, "__DATA", "__help_h32", &size);
    start = data;
    end = data + size;
}
#endif

}  // namespace ui
