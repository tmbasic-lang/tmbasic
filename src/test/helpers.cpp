#include "helpers.h"

std::string readFile(std::string filename) {
    // this will be executed from the the "bin" directory
    std::ifstream stream("../src/test/programs/" + filename);
    if (!stream.is_open()) {
        return {};
    }
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}
