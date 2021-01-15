#include "../common.h"

extern uint8_t kResourcePcode[];  // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
extern uint kResourcePcode_len;

int main(int argc, const char* argv[]) {
    for (size_t i = 0; i < kResourcePcode_len; i++) {
        std::cout << static_cast<char>(kResourcePcode[i]);
    }
    return 0;
}
