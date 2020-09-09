#pragma once

#include "Object.h"
#include "Value.h"
#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <unordered_map>
#include <vector>

namespace vm {

class ObjectToValueMap : public Object {
   public:
    const std::unordered_map<boost::local_shared_ptr<Object>, Value> pairs;
    virtual Kind getKind() const;
    virtual size_t getHash() const;
    virtual bool equals(const Object& other) const;
};

}  // namespace vm
