#include "util/path.h"

// std::filesystem support requires macOS 10.15, and we are targeting 10.13, so don't use it on macOS.
// instead we will fall back to standard POSIX stuff.
#ifdef __APPLE__
#include <libgen.h>
#include <sys/stat.h>
#endif

namespace util {

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

std::string getDirectoryName(const std::string& filePath) {
#ifdef __APPLE__
    auto* str = newStr(filePath.c_str());
    std::string directoryName{ dirname(str) };
    delete str;
    return directoryName;
#else
    return std::filesystem::path{ filePath }.parent_path().string();
#endif
}

void createDirectory(const std::string& path) {
#ifdef __APPLE__
    mkdir(path.c_str(), 0755);
#else
    std::filesystem::create_directory(path);
#endif
}

std::string pathCombine(const std::string& dir, const std::string& name) {
#ifdef __APPLE__
    auto endsInSlash = name.size() > 0 && name.at(name.size() - 1) == '/';
    if (endsInSlash) {
        return dir + name;
    } else {
        return dir + "/" + name;
    }
#else
    auto path = std::filesystem::path{ dir } / name;
    return path.string();
#endif
}

}  // namespace util
