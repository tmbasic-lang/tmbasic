#pragma once

#include "../common.h"

namespace shared {

enum class ObjectType {
    kString = 1,
    kProcedureReference,
    kTimeZone,
    kRecord,

    // List
    kValueList,
    kValueListBuilder,
    kObjectList,
    kObjectListBuilder,

    // Map
    kValueToValueMap,
    kValueToValueMapBuilder,
    kValueToObjectMap,
    kValueToObjectMapBuilder,
    kObjectToValueMap,
    kObjectToValueMapBuilder,
    kObjectToObjectMap,
    kObjectToObjectMapBuilder,

    // Set
    kValueSet,
    kValueSetBuilder,
    kObjectSet,
    kObjectSetBuilder,

    // Optional
    kValueOptional,
    kObjectOptional,
};

}  // namespace shared
