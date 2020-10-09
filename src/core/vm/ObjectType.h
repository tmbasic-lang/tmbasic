#pragma once

#include "common.h"

namespace vm {

enum class ObjectType {
    kString,
    kValueList,
    kObjectList,
    kValueToValueMap,
    kValueToObjectMap,
    kObjectToValueMap,
    kObjectToObjectMap,
    kRecord,
    kProcedureReference
};

}
