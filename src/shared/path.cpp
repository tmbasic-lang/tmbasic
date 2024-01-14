#include "shared/path.h"

// std::filesystem support requires macOS 10.15, and we are targeting 10.13, so don't use it on macOS.
// instead we will fall back to standard POSIX stuff.
#ifdef __APPLE__
#include <libgen.h>
#include <sys/stat.h>
#endif

namespace shared {

std::string getFileName(const std::string& filePath) {
#ifdef __APPLE__
    auto* str = newStr(filePath.c_str());
    std::string filename{ basename(str) };
    delete str;
    return filename;
#else
    return std::filesystem::path{ filePath }.filename().string();
#endif
}

static std::string getDirectoryNameCore(const std::string& filePath) {
#ifdef __APPLE__
    auto* str = newStr(filePath.c_str());
    std::string directoryName{ dirname(str) };
    delete str;
    return directoryName;
#else
    auto str = std::filesystem::path{ filePath }.parent_path().string();
    assert(str.size() <= filePath.size());
    return str;
#endif
}

std::string getDirectoryName(const std::string& filePath) {
    auto directoryName = getDirectoryNameCore(filePath);
    if (directoryName.empty()) {
        return ".";
    }
    return directoryName;
}

void createDirectory(const std::string& path) {
#ifdef __APPLE__
    mkdir(path.c_str(), 0755);
#else
    std::filesystem::create_directory(path);
#endif
}

std::string pathCombine(const std::string& dir, const std::string& name) {
#ifdef _WIN32
    constexpr char sep = '\\';
#else
    constexpr char sep = '/';
#endif
    size_t dirLength = dir.size();
    while (dirLength > 0 && dir.at(dirLength - 1) == sep) {
        dirLength--;
    }

    size_t nameLength = name.size();
    while (nameLength > 0 && name.at(nameLength - 1) == sep) {
        nameLength--;
    }

    std::ostringstream ss;
    ss << dir.substr(0, dirLength) << sep << name.substr(0, nameLength);
    return ss.str();
}

std::string getFileNameWithoutExtension(const std::string& filePath) {
    auto filename = getFileName(filePath);
    auto dotIndex = filename.rfind('.');
    if (dotIndex == std::string::npos) {
        return filename;
    }
    return filename.substr(0, dotIndex);
}

std::string getExtension(const std::string& filePath) {
    auto filename = getFileName(filePath);
    auto dotIndex = filename.rfind('.');
    if (dotIndex == std::string::npos) {
        return "";
    }
    return filename.substr(dotIndex);
}

}  // namespace shared
