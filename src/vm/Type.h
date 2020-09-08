#pragma once

#include "vm/Kind.h"
#include <memory>
#include <optional>

namespace vm {

class Type {
   public:
    Kind kind;
    std::optional<std::string> genericPlaceholderName;
    std::optional<std::unique_ptr<Type>> listItemType;
    std::optional<std::unique_ptr<Type>> mapKeyType;
    std::optional<std::unique_ptr<Type>> mapValueType;
};

}  // namespace vm
