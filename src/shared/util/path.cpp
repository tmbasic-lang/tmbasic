#include "path.h"

namespace util {

std::string getFileName(std::string filePath) {
    std::ostringstream s;
    for (auto ch : filePath) {
        if (ch == '/' || ch == '\\') {
            s = {};
        } else {
            s << ch;
        }
    }
    return s.str();
}

}  // namespace util
