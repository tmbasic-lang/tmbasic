#pragma once

#include "../common.h"
#include "vm/Value.h"

namespace vm {

// a TimeSpan is represented as a Value counting milliseconds
class TimeSpan {
   public:
    static Value fromDays(const Value& days);
    static Value fromHours(const Value& days);
    static Value fromMinutes(const Value& days);
    static Value fromSeconds(const Value& days);

   private:
    TimeSpan();
};

}  // namespace vm