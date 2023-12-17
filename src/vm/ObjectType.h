#pragma once

#include "../common.h"

namespace vm {

enum class ObjectType {
    kString = 1,
    kValueList,
    kValueListBuilder,
    kObjectList,
    kObjectListBuilder,
    kValueToValueMap,
    kValueToValueMapBuilder,
    kValueToObjectMap,
    kValueToObjectMapBuilder,
    kObjectToValueMap,
    kObjectToValueMapBuilder,
    kObjectToObjectMap,
    kObjectToObjectMapBuilder,
    kRecord,
    kProcedureReference,
    kValueOptional,
    kObjectOptional,
    kTimeZone,
};

}  // namespace vm
