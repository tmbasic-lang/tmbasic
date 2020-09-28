#pragma once

#include "common.h"
#include "Kind.h"

namespace vm {

class Type {
   public:
    Kind kind;
    std::optional<std::string> genericPlaceholderName;
    std::optional<std::unique_ptr<Type>> listItemType;
    std::optional<std::unique_ptr<Type>> mapKeyType;
    std::optional<std::unique_ptr<Type>> mapValueType;
    std::optional<std::unique_ptr<Type>> optionalValueType;
};

}  // namespace vm
