#include "Value.h"

using namespace vm;

bool Value::operator==(const Value& rhs) const {
    return bits == rhs.bits;
}

std::size_t ValueHash::operator()(const Value& k) const {
    return std::hash<uint64_t>{}(k.bits);
}
