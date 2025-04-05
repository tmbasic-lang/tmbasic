#include "helpers.h"

using std::ifstream;
using std::ostringstream;
using std::string;
using std::vector;

string readFile(const string& filename) {
    // this will be executed from the repository root.
    ifstream stream("src/test/programs/" + filename);
    if (!stream.is_open()) {
        return {};
    }
    ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

vector<uint8_t> readBinaryFile(const string& filename) {
    // this will be executed from the repository root.
    ifstream stream("src/test/files/" + filename, ios::binary | ios::ate);
    auto pos = stream.tellg();
    vector<uint8_t> result(pos);
    stream.seekg(0, ios::beg);
    stream.read(reinterpret_cast<char*>(result.data()), pos);
    return result;
}
