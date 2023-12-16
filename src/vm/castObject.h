#pragma once

#include "../common.h"
#include "vm/List.h"
#include "vm/Map.h"
#include "vm/Object.h"
#include "vm/Optional.h"
#include "vm/Record.h"
#include "vm/String.h"
#include "vm/TimeZone.h"

namespace vm {

// In release builds, all of the below collapses down to a dynamic_cast.
// In debug builds, it provides additional information on bad casts.

template <typename TObject>
inline TObject& castObject(Object& obj, ObjectType objectType) {
#ifdef NDEBUG
    (void)objectType;
#else
    if (obj.getObjectType() != objectType) {
        std::cerr << "Internal type confusion. Expected: " << NAMEOF_ENUM(objectType)
                  << ". Actual: " << NAMEOF_ENUM(obj.getObjectType()) << "." << std::endl;
    }
#endif

    return dynamic_cast<TObject&>(obj);
}

template <typename TObject>
inline const TObject& castObject(const Object& obj, ObjectType objectType) {
#ifdef NDEBUG
    (void)objectType;
#else
    if (obj.getObjectType() != objectType) {
        std::cerr << "Internal type confusion. Expected: " << NAMEOF_ENUM(objectType)
                  << ". Actual: " << NAMEOF_ENUM(obj.getObjectType()) << "." << std::endl;
    }
#endif

    return dynamic_cast<const TObject&>(obj);
}

inline String& castString(vm::Object& obj) {
    return castObject<String>(obj, ObjectType::kString);
}

inline ValueList& castValueList(vm::Object& obj) {
    return castObject<ValueList>(obj, ObjectType::kValueList);
}

inline ValueListBuilder& castValueListBuilder(vm::Object& obj) {
    return castObject<ValueListBuilder>(obj, ObjectType::kValueListBuilder);
}

inline ObjectList& castObjectList(vm::Object& obj) {
    return castObject<ObjectList>(obj, ObjectType::kObjectList);
}

inline ObjectListBuilder& castObjectListBuilder(vm::Object& obj) {
    return castObject<ObjectListBuilder>(obj, ObjectType::kObjectListBuilder);
}

inline ValueToValueMap& castValueToValueMap(vm::Object& obj) {
    return castObject<ValueToValueMap>(obj, ObjectType::kValueToValueMap);
}

inline ValueToObjectMap& castValueToObjectMap(vm::Object& obj) {
    return castObject<ValueToObjectMap>(obj, ObjectType::kValueToObjectMap);
}

inline ObjectToValueMap& castObjectToValueMap(vm::Object& obj) {
    return castObject<ObjectToValueMap>(obj, ObjectType::kObjectToValueMap);
}

inline ObjectToObjectMap& castObjectToObjectMap(vm::Object& obj) {
    return castObject<ObjectToObjectMap>(obj, ObjectType::kObjectToObjectMap);
}

inline Record& castRecord(vm::Object& obj) {
    return castObject<Record>(obj, ObjectType::kRecord);
}

inline ValueOptional& castValueOptional(vm::Object& obj) {
    return castObject<ValueOptional>(obj, ObjectType::kValueOptional);
}

inline ObjectOptional& castObjectOptional(vm::Object& obj) {
    return castObject<ObjectOptional>(obj, ObjectType::kObjectOptional);
}

inline TimeZone& castTimeZone(vm::Object& obj) {
    return castObject<TimeZone>(obj, ObjectType::kTimeZone);
}

// Const versions of the above.

inline const String& castString(const vm::Object& obj) {
    return castObject<String>(obj, ObjectType::kString);
}

inline const ValueList& castValueList(const vm::Object& obj) {
    return castObject<ValueList>(obj, ObjectType::kValueList);
}

inline const ValueListBuilder& castValueListBuilder(const vm::Object& obj) {
    return castObject<ValueListBuilder>(obj, ObjectType::kValueListBuilder);
}

inline const ObjectList& castObjectList(const vm::Object& obj) {
    return castObject<ObjectList>(obj, ObjectType::kObjectList);
}

inline const ObjectListBuilder& castObjectListBuilder(const vm::Object& obj) {
    return castObject<ObjectListBuilder>(obj, ObjectType::kObjectListBuilder);
}

inline const ValueToValueMap& castValueToValueMap(const vm::Object& obj) {
    return castObject<ValueToValueMap>(obj, ObjectType::kValueToValueMap);
}

inline const ValueToObjectMap& castValueToObjectMap(const vm::Object& obj) {
    return castObject<ValueToObjectMap>(obj, ObjectType::kValueToObjectMap);
}

inline const ObjectToValueMap& castObjectToValueMap(const vm::Object& obj) {
    return castObject<ObjectToValueMap>(obj, ObjectType::kObjectToValueMap);
}

inline const ObjectToObjectMap& castObjectToObjectMap(const vm::Object& obj) {
    return castObject<ObjectToObjectMap>(obj, ObjectType::kObjectToObjectMap);
}

inline const Record& castRecord(const vm::Object& obj) {
    return castObject<Record>(obj, ObjectType::kRecord);
}

inline const ValueOptional& castValueOptional(const vm::Object& obj) {
    return castObject<ValueOptional>(obj, ObjectType::kValueOptional);
}

inline const ObjectOptional& castObjectOptional(const vm::Object& obj) {
    return castObject<ObjectOptional>(obj, ObjectType::kObjectOptional);
}

inline const TimeZone& castTimeZone(const vm::Object& obj) {
    return castObject<TimeZone>(obj, ObjectType::kTimeZone);
}

}  // namespace vm
