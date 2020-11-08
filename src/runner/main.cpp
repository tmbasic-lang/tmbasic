#include "common.h"

static uint8_t* findAppendedData() {
    auto pid = getpid();
    auto s = std::ostringstream();
    s << "/proc/" << pid << "/exe";
    auto exeFilePath = s.str();
    auto file = std::ifstream(exeFilePath);

    // the last three bytes are "TM\n"
    file.seekg(-3, std::ios::end);
    char sentinel[3] = { 0 };
    file.read(sentinel, 3);
    if (sentinel[0] != 'T' || sentinel[1] != 'M' || sentinel[2] != '\n') {
        std::cerr << "Cannot find a compiled program in this executable file." << std::endl;
        exit(-1);
        return nullptr;
    }

    //TODO
    return nullptr;
}

int main(int argc, char** argv) {
    findAppendedData();
    return 0;
}
