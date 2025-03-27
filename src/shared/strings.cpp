#include "strings.h"

namespace shared {

std::string to_lower_copy(const std::string& str) {
    std::string result;
    for (char const c : str) {
        result += std::tolower(c);
    }
    return result;
}

// There's another copy of this function in buildDoc.cpp
std::string trim_copy(const std::string& str) {
    if (str.empty()) {
        return str;
    }

    // Find first non-whitespace character
    size_t const start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return "";  // String contains only whitespace
    }

    // Find last non-whitespace character
    size_t const end = str.find_last_not_of(" \t\n\r\f\v");

    // Return the trimmed substring
    return str.substr(start, end - start + 1);
}

std::string replace_all_copy(const std::string& haystack, const std::string& needle, const std::string& replacement) {
    std::string result = haystack;
    replace_all(&result, needle, replacement);
    return result;
}

void replace_all(std::string* haystack, const std::string& needle, const std::string& replacement) {
    if (needle.empty() || !haystack) {
        return;
    }

    size_t pos = 0;
    while ((pos = haystack->find(needle, pos)) != std::string::npos) {
        haystack->replace(pos, needle.length(), replacement);
        pos += replacement.length();
    }
}

}  // namespace shared
