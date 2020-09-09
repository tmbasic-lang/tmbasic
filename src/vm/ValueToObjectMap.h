#pragma once

#include "Object.h"
#include "Value.h"
#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <unordered_map>

namespace vm {

class ValueToObjectMap : public Object {
   public:
    const std::unordered_map<Value, boost::local_shared_ptr<Object>> pairs;
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}  // namespace vm
