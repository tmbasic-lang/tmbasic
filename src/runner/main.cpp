#include "../common.h"
#include "shared/ExecutablePatcher.h"
#include "shared/console.h"
#include "shared/process.h"
#include "vm/BasicApp.h"
#include "vm/Interpreter.h"
#include "vm/Program.h"
#include "vm/date.h"

static void unpackResources(vm::Program* program, std::vector<char>* tzdb) {
    const auto exeFilePath = shared::getExecutableFilePath();
    shared::ExecutablePatcher patcher{ exeFilePath };

    auto tzdbUint8 = patcher.getResource(shared::ExecutablePatcher::kResourceTzdb);
    tzdb->assign(tzdbUint8.begin(), tzdbUint8.end());

    auto pcode = patcher.getResource(shared::ExecutablePatcher::kResourcePcode);
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
