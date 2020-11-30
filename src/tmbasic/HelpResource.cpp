#include "HelpResource.h"

using tmbasic::HelpResource;

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <mach-o/getsect.h>
#include <mach-o/ldsyms.h>
HelpResource::HelpResource() {
    size_t size = 0;
    auto data = getsectiondata(&_mh_execute_header, "__DATA", "__help_h32", &size);
    start = data;
    end = data + size;
}
#endif

#ifdef __linux__
extern char _binary_help_h32_start[];
extern char _binary_help_h32_end[];
HelpResource::HelpResource()
    : start(reinterpret_cast<uint8_t*>(_binary_help_h32_start)),
      end(reinterpret_cast<uint8_t*>(_binary_help_h32_end)) {}
#endif

#ifdef _WIN32
HelpResource::HelpResource() {
    auto resourceInfoHandle = FindResource(nullptr, MAKEINTRESOURCE(1000), RT_RCDATA);
    if (!resourceInfoHandle) {
        throw std::runtime_error("FindResource failed!");
    }
    auto resourceHandle = LoadResource(nullptr, resourceInfoHandle);
    if (!resourceHandle) {
        throw std::runtime_error("LoadResource failed!");
    }
    auto data = reinterpret_cast<uint8_t*>(LockResource(resourceHandle));
    if (!data) {
        throw std::runtime_error("LockResource failed!");
    }
    start = data;
    auto size = SizeofResource(nullptr, resourceInfoHandle);
    if (size == 0) {
        throw std::runtime_error("SizeofResource failed!");
    }
    end = start + size;
}
#endif
