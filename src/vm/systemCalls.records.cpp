#include "vm/systemCall.h"
#include "vm/castObject.h"
#include "vm/Record.h"

namespace vm {

// (lhs as Record, rhs as Record) as Boolean
void systemCallRecordEquals(const SystemCallInput& input, SystemCallResult* result) {
    const auto& lhs = *castRecord(input.getObject(-2));
    const auto& rhs = *castRecord(input.getObject(-1));
    result->returnedValue = Value{ lhs.equals(rhs) };
}

// (lhs as Record, rhs as Record) as Boolean
void systemCallRecordNotEquals(const SystemCallInput& input, SystemCallResult* result) {
    const auto& lhs = *castRecord(input.getObject(-2));
    const auto& rhs = *castRecord(input.getObject(-1));
    result->returnedValue = Value{ !lhs.equals(rhs) };
}

}  // namespace vm
