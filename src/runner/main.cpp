#include "../common.h"
#include "shared/console.h"
#include "vm/BasicApp.h"
#include "vm/Interpreter.h"
#include "vm/Program.h"
#include "vm/date.h"

extern const uint8_t kResourcePcode[];  // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
extern const uint kResourcePcode_len;

static void deserializePcode(vm::Program* program) {
    std::vector<uint8_t> pcode{};
    pcode.reserve(kResourcePcode_len);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    pcode.insert(pcode.end(), kResourcePcode, kResourcePcode + kResourcePcode_len);
    program->deserialize(pcode);
}

int main(int /*argc*/, const char* /*argv*/[]) {
    try {
        shared::setUtf8Locale();
        vm::initializeTzdb();

        vm::Program program{};
        deserializePcode(&program);

        vm::Interpreter interpreter{ &program, &std::cin, &std::cout };
        interpreter.init(program.startupProcedureIndex);
        while (interpreter.run(10000)) {
        }

        auto error = interpreter.getError();
        if (error.has_value()) {
            std::cerr << "Error " << error->code.getString() << ": " << error->message << std::endl;
            return 1;
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error." << std::endl;
        return 1;
    }
}
