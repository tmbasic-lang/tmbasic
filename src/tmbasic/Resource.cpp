#include "Resource.h"

using std::string;
using tmbasic::Resource;
using tmbasic::ResourceData;

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <mach-o/getsect.h>
#include <mach-o/ldsyms.h>

static ResourceData getResourceCore(Resource resource, string macName, int winId) {
    size_t size = 0;
    auto data = getsectiondata(&_mh_execute_header, "__DATA", macName.c_str(), &size);
    ResourceData result;
    result.start = data;
    result.end = data + size;
    return result;
}
#endif

#ifdef _WIN32
static ResourceData getResourceCore(Resource resource, string macName, int winId) {
    auto resourceInfoHandle = FindResource(nullptr, MAKEINTRESOURCE(winId), RT_RCDATA);
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
    ResourceData result;
    result.start = data;
    auto size = SizeofResource(nullptr, resourceInfoHandle);
    if (size == 0) {
        throw std::runtime_error("SizeofResource failed!");
    }
    result.end = result.start + size;
    return result;
}
#endif

#ifdef __linux__
extern uint8_t _binary_help_h32_start[];  // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
extern uint8_t _binary_help_h32_end[];    // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)

extern uint8_t _binary_runner_linux_arm32_start[];  // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
extern uint8_t _binary_runner_linux_arm32_end[];    // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)

extern uint8_t _binary_runner_linux_arm64_start[];  // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
extern uint8_t _binary_runner_linux_arm64_end[];    // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)

extern uint8_t _binary_runner_linux_x64_start[];  // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
extern uint8_t _binary_runner_linux_x64_end[];    // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)

extern uint8_t _binary_runner_linux_x86_start[];  // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
extern uint8_t _binary_runner_linux_x86_end[];    // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)

extern uint8_t _binary_runner_mac_x64_start[];  // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
extern uint8_t _binary_runner_mac_x64_end[];    // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)

extern uint8_t _binary_runner_win_x64_start[];  // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
extern uint8_t _binary_runner_win_x64_end[];    // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)

extern uint8_t _binary_runner_win_x86_start[];  // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
extern uint8_t _binary_runner_win_x86_end[];    // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)

static ResourceData getResourceCore(Resource resource, const string& /*macName*/, int /*winId*/) {
    switch (resource) {
        case Resource::kHelp:
            return { static_cast<uint8_t*>(_binary_help_h32_start), static_cast<uint8_t*>(_binary_help_h32_end) };
        default:
            assert(false);
            return {};
    }
}
#endif

namespace tmbasic {

ResourceData getResource(Resource resource) {
    switch (resource) {
        case Resource::kHelp:
            return getResourceCore(resource, "__help_h32", 1000);
        default:
            assert(false);
            return {};
    }
}

}  // namespace tmbasic
