#include "../common.h"
#include "shared/console.h"
#include "shared/process.h"
#include "shared/runner_const.h"
#include "vm/BasicApp.h"
#include "vm/Interpreter.h"
#include "vm/Program.h"
#include "vm/date.h"

static void unpackResources(vm::Program* program, std::vector<char>* tzdb) {
    const auto exeFilePath = shared::getExecutableFilePath();

    std::ifstream exeFile(exeFilePath, std::ios::binary);

    // The last 4 bytes are a sentinel value: kPcodeSentinel.
    exeFile.seekg(-4, std::ios::end);
    uint32_t sentinel{};
    exeFile.read(reinterpret_cast<char*>(&sentinel), sizeof(sentinel));
    if (sentinel != shared::kPcodeSentinel) {
        throw std::runtime_error("The runner executable is corrupted.");
    }

    // The preceding 4 bytes are the tzdb length (NOT including the length itself)
    exeFile.seekg(-8, std::ios::end);
    uint32_t tzdbLength{};
    exeFile.read(reinterpret_cast<char*>(&tzdbLength), sizeof(tzdbLength));
    std::cerr << "tzdbLength: " << tzdbLength << '\n';

    // Read the tzdb data immediately preceding the length
    tzdb->resize(tzdbLength);
    exeFile.seekg(-8 - tzdbLength, std::ios::end);
    exeFile.read(reinterpret_cast<char*>(tzdb->data()), tzdbLength);

    // The preceding 4 bytes are the length of the pcode (NOT including the length itself)
    exeFile.seekg(-12 - tzdbLength, std::ios::end);
    uint32_t pcodeLength{};
    exeFile.read(reinterpret_cast<char*>(&pcodeLength), sizeof(pcodeLength));
    std::cerr << "pcodeLength: " << pcodeLength << '\n';
    // Now read the pcode immediately preceding the length.
    std::vector<uint8_t> pcode(pcodeLength);
    exeFile.seekg(-12 - tzdbLength - pcodeLength, std::ios::end);
    exeFile.read(reinterpret_cast<char*>(pcode.data()), pcodeLength);

    program->deserialize(pcode);
}

int main(int /*argc*/, const char* /*argv*/[]) {
    try {
        shared::setUtf8Locale();

        vm::Program program{};
        std::vector<char> tzdb{};
        unpackResources(&program, &tzdb);

        vm::initializeTzdbFromBuffer(tzdb);

        vm::Interpreter interpreter{ &program, &std::cin, &std::cout };
        interpreter.init(program.startupProcedureIndex);
        while (interpreter.run(10000)) {
        }

        auto error = interpreter.getError();
        if (error.has_value()) {
            std::cerr << "Error " << error->code.getString() << ": " << error->message << '\n';
            return 1;
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error." << '\n';
        return 1;
    }
}
