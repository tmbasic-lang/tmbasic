#include "process.h"

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#include <libgen.h>
#include <string.h>
#else                  // Linux
#include <sys/auxv.h>  // For getauxval()
#include <climits>
#include <cstdlib>
#endif

#include <stdexcept>
#if !defined(__APPLE__)
#include <filesystem>
#endif

namespace shared {

std::string getExecutableFilePath() {
#ifdef _WIN32
    // Windows implementation
    wchar_t path[MAX_PATH];
    DWORD result = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (result == 0) {
        throw std::runtime_error("Failed to get executable path");
    }

    std::filesystem::path exePath(path);
    return exePath.string();
#elif defined(__APPLE__)
    // macOS implementation
    char pathbuf[PATH_MAX];
    uint32_t bufsize = PATH_MAX;
    if (_NSGetExecutablePath(pathbuf, &bufsize) != 0) {
        throw std::runtime_error("Failed to get executable path");
    }

    // Convert to absolute path
    char realpathbuf[PATH_MAX];
    if (realpath(pathbuf, realpathbuf) == nullptr) {
        throw std::runtime_error("Failed to resolve executable path");
    }

    return std::string(realpathbuf);
#else
    // Linux implementation for statically linked executable
    // NOLINTNEXTLINE(performance-no-int-to-ptr)
    const char* exe = reinterpret_cast<const char*>(getauxval(AT_EXECFN));
    if (!exe) {
        throw std::runtime_error("Failed to get executable path");
    }

    // Convert to absolute path if needed
    std::array<char, PATH_MAX> realpathbuf{};
    if (realpath(exe, realpathbuf.data()) == nullptr) {
        throw std::runtime_error("Failed to resolve executable path");
    }

    return { realpathbuf.data() };
#endif
}

std::string getExecutableDirectoryPath() {
    std::string const exePath = getExecutableFilePath();

#ifdef __APPLE__
    // Use dirname for macOS
    std::array<char, PATH_MAX> dirpath{};
    strncpy(dirpath.data(), exePath.c_str(), PATH_MAX);
    return { dirname(dirpath.data()) };
#else
    // Use std::filesystem for Windows and Linux
    std::filesystem::path const path(exePath);
    return path.parent_path().string();
#endif
}

}  // namespace shared
