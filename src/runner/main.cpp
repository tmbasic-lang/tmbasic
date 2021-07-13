#include "../common.h"
#include "vm/Interpreter.h"
#include "vm/Program.h"

extern uint8_t kResourcePcode[];  // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
extern uint kResourcePcode_len;

int main(int /*argc*/, const char* /*argv*/[]) {
    std::vector<uint8_t> pcode{};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    pcode.insert(pcode.end(), kResourcePcode, kResourcePcode + kResourcePcode_len);
    vm::Program program{};
    program.deserialize(pcode);
    vm::Interpreter interpreter{ &program, &std::cin, &std::cout };
    interpreter.init(program.startupProcedureIndex);
    while (interpreter.run(10000)) {
    }
    return 0;
}
