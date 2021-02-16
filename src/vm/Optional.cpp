#include "Optional.h"

namespace vm {

ValueOptional::ValueOptional() = default;

ValueOptional::ValueOptional(Value value) : item(std::move(value)) {}

ObjectType ValueOptional::getObjectType() const {
    return ObjectType::kValueOptional;
}

size_t ValueOptional::getHash() const {
    return item.has_value() ? item.value().getHash() : 0;
}

bool ValueOptional::equals(const Object& other) const {
    if (other.getObjectType() != ObjectType::kValueOptional) {
        return false;
    }
    const auto& otherOptional = dynamic_cast<const ValueOptional&>(other);
    return item.has_value() == otherOptional.item.has_value() &&
        (item.has_value() ? item.value() == otherOptional.item.value() : true);
}

ObjectOptional::ObjectOptional() = default;

ObjectOptional::ObjectOptional(boost::local_shared_ptr<Object> object) : item(std::move(object)) {}

ObjectType ObjectOptional::getObjectType() const {
    return ObjectType::kObjectOptional;
}

size_t ObjectOptional::getHash() const {
    return item.has_value() ? item.value()->getHash() : 0;
}

bool ObjectOptional::equals(const Object& other) const {
    if (other.getObjectType() != ObjectType::kObjectOptional) {
        return false;
    }
    const auto& otherOptional = dynamic_cast<const ObjectOptional&>(other);
    return item.has_value() == otherOptional.item.has_value() &&
        (item.has_value() ? item.value()->equals(*otherOptional.item.value()) : true);
}

}  // namespace vm
