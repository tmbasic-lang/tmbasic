#include "util/path.h"

#ifdef __APPLE__
#include <libgen.h>
#include <sys/stat.h>
#endif

namespace util {

std::string getFileName(const std::string& filePath) {
    std::ostringstream s;
    for (auto ch : filePath) {
        if (ch == '/' || ch == '\\') {
            s = std::ostringstream();
        } else {
            s << ch;
        }
    }
    return s.str();
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

}  // namespace util
