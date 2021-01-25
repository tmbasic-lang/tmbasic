#include <cstdio>
#include <fstream>
#include <regex>
#include <string>

using std::ofstream;
using std::regex;
using std::regex_search;
using std::smatch;
using std::string;

static size_t getFileSize(string filePath) {
    auto* fp = fopen(filePath.c_str(), "rb");
    if (!fp) {
        return false;
    }
    fseek(fp, 0, SEEK_END);
    auto length = ftell(fp);
    fclose(fp);
    return length;
}

static string replace(string haystack, string needle, string replacement) {
    auto pos = haystack.find(needle);
    while (pos != string::npos) {
        haystack.replace(pos, needle.length(), replacement);
        pos = haystack.find(needle, pos + replacement.length());
    }
    return haystack;
}

static string getConstName(string filePath) {
    regex basenameRegex("[^/]+$");
    smatch basenameMatch;
    if (!regex_search(filePath, basenameMatch, basenameRegex)) {
        throw std::runtime_error("Filename doesn't match pattern.");
    }

    return string("kRunnerLength_") + replace(basenameMatch.str(), ".", "_");
}

int main(int argc, const char* argv[]) {
    // argv[1] = platform (e.g. linux_arm64)

    auto outputStream = ofstream("obj/resources/runners/runners.h");
    outputStream << "#pragma once\n";
    outputStream << "#include <cstddef>\n";
    for (auto i = 1; i < argc; i++) {
        auto size = getFileSize(argv[i]);
        auto name = getConstName(argv[i]);
        outputStream << "constexpr size_t " << name << " = " << size << ";\n";
    }
    return 0;
}
