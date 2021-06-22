#pragma once

#include "../common.h"

namespace vm {

enum class ObjectType {
    kString = 1,
    kValueList,
    kObjectList,
    kValueToValueMap,
    kValueToObjectMap,
    kObjectToValueMap,
    kObjectToObjectMap,
    kRecord,
    kProcedureReference,
    kValueOptional,
    kObjectOptional,
    kTimeZone,
};

}  // namespace vm
