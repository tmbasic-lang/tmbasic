#include "vm/systemCall.h"

namespace vm {

// (delay as TimeSpan)
void systemCallSleep(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto milliseconds = input.getValue(-1).getInt64();
    if (milliseconds > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }
}

}  // namespace vm
