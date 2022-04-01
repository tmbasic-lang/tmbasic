#include "systemCall.h"
#include "Map.h"

namespace vm {

void initSystemCallsMaps() {
    initSystemCall(SystemCall::kObjectToObjectMapNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ObjectToObjectMap>();
    });

    initSystemCall(SystemCall::kObjectToValueMapNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ObjectToValueMap>();
    });

    initSystemCall(SystemCall::kValueToObjectMapNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ValueToObjectMap>();
    });

    initSystemCall(SystemCall::kValueToValueMapNew, [](const auto& /*input*/, auto* result) {
        result->returnedObject = boost::make_local_shared<ValueToValueMap>();
    });
}

}  // namespace vm
