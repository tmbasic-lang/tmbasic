#pragma once
#include <cstdint>
#include <unordered_map>

namespace vm {

union Value {
    bool boolean;
    int64_t integer;
    double decimal;
    uint64_t bits;

    bool operator==(const Value& rhs) const;
};

struct ValueHash {
    std::size_t operator()(const Value& k) const;
};

}  // namespace vm
