#include "Optional.h"
#include "castObject.h"

using shared::ObjectType;

namespace vm {

ValueOptional::ValueOptional() = default;

ValueOptional::ValueOptional(Value value) : item(std::move(value)) {}

ObjectType ValueOptional::getObjectType() const {
    return ObjectType::kValueOptional;
}

size_t ValueOptional::getHash() const {
    return item.has_value() ? (*item).getHash() : 0;
}

bool ValueOptional::equals(const Object& other) const {
    if (other.getObjectType() != ObjectType::kValueOptional) {
        return false;
    }
    const auto& otherOptional = castValueOptional(other);
    return item.has_value() == otherOptional.item.has_value() &&
        (item.has_value() ? *item == *otherOptional.item : true);
}

ObjectOptional::ObjectOptional() = default;

ObjectOptional::ObjectOptional(boost::intrusive_ptr<Object> object) : item(std::move(object)) {}

ObjectType ObjectOptional::getObjectType() const {
    return ObjectType::kObjectOptional;
}

size_t ObjectOptional::getHash() const {
    return item.has_value() ? (*item)->getHash() : 0;
}

bool ObjectOptional::equals(const Object& other) const {
    if (other.getObjectType() != ObjectType::kObjectOptional) {
        return false;
    }
    const auto& otherOptional = castObjectOptional(other);
    return item.has_value() == otherOptional.item.has_value() &&
        (item.has_value() ? (*item)->equals(**otherOptional.item) : true);
}

}  // namespace vm
