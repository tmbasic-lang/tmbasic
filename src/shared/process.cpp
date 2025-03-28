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
#include <limits.h>
#include <stdlib.h>
#endif

#include <stdexcept>
#if !defined(__APPLE__)
#include <filesystem>
#endif

namespace shared {

std::string getExecutableDirectoryPath() {
#ifdef _WIN32
    // Windows implementation
    wchar_t path[MAX_PATH];
    DWORD result = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (result == 0) {
        throw std::runtime_error("Failed to get executable path");
    }

    std::filesystem::path exePath(path);
    return exePath.parent_path().string();
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

    // Use dirname to get directory path (make a copy since dirname can modify the string)
    char dirpath[PATH_MAX];
    strncpy(dirpath, realpathbuf, PATH_MAX);
    return std::string(dirname(dirpath));
#else
    // Linux implementation for statically linked executable
    const char* exe = reinterpret_cast<const char*>(getauxval(AT_EXECFN));
    if (!exe) {
        throw std::runtime_error("Failed to get executable path");
    }

    // Convert to absolute path if needed
    char realpathbuf[PATH_MAX];
    if (realpath(exe, realpathbuf) == nullptr) {
        throw std::runtime_error("Failed to resolve executable path");
    }

    std::filesystem::path exePath(realpathbuf);
    return exePath.parent_path().string();
#endif
}

}  // namespace shared
