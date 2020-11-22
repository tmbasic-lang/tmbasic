#include "shared/util/path.h"

namespace util {

std::string getFileName(std::string filePath) {
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

}  // namespace util
