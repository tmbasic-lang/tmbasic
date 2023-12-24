#pragma once

#include "../common.h"
#include "vm/List.h"
#include "vm/Map.h"
#include "vm/Object.h"
#include "vm/Optional.h"
#include "vm/ProcedureReference.h"
#include "vm/Record.h"
#include "vm/Set.h"
#include "vm/String.h"
#include "vm/TimeZone.h"

namespace vm {

// In release builds, all of the below collapses down to a reinterpret_cast.
// In debug builds, it provides additional information on bad casts.

template <typename TObject>
inline TObject* castObject(Object* obj, ObjectType objectType) {
#ifdef NDEBUG
    (void)objectType;
    return reinterpret_cast<TObject*>(obj);
#else
    if (obj->getObjectType() != objectType) {
        std::cerr << "Internal type confusion. Expected: " << NAMEOF_ENUM(objectType)
                  << ". Actual: " << NAMEOF_ENUM(obj->getObjectType()) << "." << std::endl;
        assert(false);
    }
    return reinterpret_cast<TObject*>(obj);
#endif
}

template <typename TObject>
inline const TObject& castObject(const Object& obj, ObjectType objectType) {
#ifdef NDEBUG
    (void)objectType;
    return reinterpret_cast<const TObject&>(obj);
#else
    if (obj.getObjectType() != objectType) {
        std::cerr << "Internal type confusion. Expected: " << NAMEOF_ENUM(objectType)
                  << ". Actual: " << NAMEOF_ENUM(obj.getObjectType()) << "." << std::endl;
        assert(false);
    }
    return reinterpret_cast<const TObject&>(obj);
#endif
}

#define DEFINE_CAST_FUNCTION(TYPE)                                                                  \
    inline TYPE* cast##TYPE(vm::Object* obj) { return castObject<TYPE>(obj, ObjectType::k##TYPE); } \
    inline const TYPE& cast##TYPE(const vm::Object& obj) { return castObject<TYPE>(obj, ObjectType::k##TYPE); }

DEFINE_CAST_FUNCTION(ObjectList)
DEFINE_CAST_FUNCTION(ObjectListBuilder)
DEFINE_CAST_FUNCTION(ObjectOptional)
DEFINE_CAST_FUNCTION(ObjectSet)
DEFINE_CAST_FUNCTION(ObjectSetBuilder)
DEFINE_CAST_FUNCTION(ObjectToObjectMap)
DEFINE_CAST_FUNCTION(ObjectToObjectMapBuilder)
DEFINE_CAST_FUNCTION(ObjectToValueMap)
DEFINE_CAST_FUNCTION(ObjectToValueMapBuilder)
DEFINE_CAST_FUNCTION(ProcedureReference)
DEFINE_CAST_FUNCTION(Record)
DEFINE_CAST_FUNCTION(String)
DEFINE_CAST_FUNCTION(TimeZone)
DEFINE_CAST_FUNCTION(ValueList)
DEFINE_CAST_FUNCTION(ValueListBuilder)
DEFINE_CAST_FUNCTION(ValueOptional)
DEFINE_CAST_FUNCTION(ValueSet)
DEFINE_CAST_FUNCTION(ValueSetBuilder)
DEFINE_CAST_FUNCTION(ValueToObjectMap)
DEFINE_CAST_FUNCTION(ValueToObjectMapBuilder)
DEFINE_CAST_FUNCTION(ValueToValueMap)
DEFINE_CAST_FUNCTION(ValueToValueMapBuilder)

#undef DEFINE_CAST_FUNCTION

}  // namespace vm
