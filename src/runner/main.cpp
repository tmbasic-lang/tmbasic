#include "../common.h"
#include "util/console.h"
#include "vm/BasicApp.h"
#include "vm/Interpreter.h"
#include "vm/Program.h"
#include "vm/date.h"

extern const uint8_t kResourcePcode[];  // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
extern const uint kResourcePcode_len;

int main(int /*argc*/, const char* /*argv*/[]) {
    util::setUtf8Locale();
    vm::initializeTzdb();

    std::vector<uint8_t> pcode{};
    pcode.reserve(kResourcePcode_len);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    pcode.insert(pcode.end(), kResourcePcode, kResourcePcode + kResourcePcode_len);

    vm::Program program{};
    program.deserialize(pcode);

    vm::BasicApp app{};
    app.suspend();

    vm::Interpreter interpreter{ &program, &std::cin, &std::cout };
    interpreter.init(program.startupProcedureIndex);
    while (interpreter.run(10000)) {
    }

    app.shutDown();
    return 0;
}
