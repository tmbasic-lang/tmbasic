#pragma once

namespace vm {

enum class Kind {
    kBoolean,
    kInteger,
    kDecimal,
    kString,
    kValueList,
    kObjectList,
    kValueToValueMap,
    kValueToObjectMap,
    kObjectToValueMap,
    kObjectToObjectMap,
    kRecord,
    kProcedureReference,
    kGenericPlaceholder,
    kOptional
};

}
