#include "RecordBuilder.h"

namespace vm {

RecordBuilder::RecordBuilder(int numValues, int numObjects)
    : values(immer::array<Value>(numValues, Value()).transient()),
      objects(immer::array<boost::local_shared_ptr<Object>>(numObjects, nullptr).transient()) {}

}  // namespace vm
