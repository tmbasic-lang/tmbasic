#include "vm/systemCall.h"
#include "shared/Error.h"
#include "vm/List.h"
#include "vm/String.h"
#include "vm/castObject.h"

using shared::Error;
using shared::ErrorCode;

namespace vm {

static std::pair<ValueList*, ObjectList*> valueOrObjectList(Object* object) {
    auto type = object->getObjectType();

    if (type == ObjectType::kValueList) {
        return { castValueList(object), nullptr };
    }

    if (type == ObjectType::kObjectList) {
        return { nullptr, castObjectList(object) };
    }

    throw Error(
        ErrorCode::kInternalTypeConfusion,
        fmt::format(
            "Internal type confusion error. Target is neither {} nor {}.", NAMEOF_TYPE(ValueList),
            NAMEOF_TYPE(ObjectList)));
}

static void listFirstOrLast(const SystemCallInput& input, SystemCallResult* result, bool first) {
    const auto valueOrObject = valueOrObjectList(input.getObject(-1));
    const auto* valueList = valueOrObject.first;
    const auto* objectList = valueOrObject.second;

    if (valueList != nullptr) {
        if (valueList->items.empty()) {
            throw Error(ErrorCode::kListIsEmpty, "List is empty.");
        }
        result->returnedValue = first ? valueList->items.front() : valueList->items.back();
    } else {
        if (objectList->items.empty()) {
            throw Error(ErrorCode::kListIsEmpty, "List is empty.");
        }
        result->returnedObject = first ? objectList->items.front() : objectList->items.back();
    }
}

static void listSkipOrTake(const SystemCallInput& input, SystemCallResult* result, bool skip) {
    auto countSigned = input.getValue(-1).getInt64();
    if (countSigned < 0) {
        throw Error(ErrorCode::kInvalidArgument, "Count must be non-negative.");
    }
    auto count = static_cast<size_t>(countSigned);

    const auto valueOrObject = valueOrObjectList(input.getObject(-1));
    const auto* valueList = valueOrObject.first;
    const auto* objectList = valueOrObject.second;

    if (valueList != nullptr) {
        ValueListBuilder builder{};
        if (skip) {
            for (size_t i = count; i < valueList->items.size(); i++) {
                builder.items.push_back(valueList->items.at(i));
            }
        } else {
            for (size_t i = 0; i < count && i < valueList->items.size(); i++) {
                builder.items.push_back(valueList->items.at(i));
            }
        }
        result->returnedObject = boost::make_intrusive_ptr<ValueList>(&builder);
    } else {
        ObjectListBuilder builder{};
        if (skip) {
            for (size_t i = count; i < objectList->items.size(); i++) {
                builder.items.push_back(objectList->items.at(i));
            }
        } else {
            for (size_t i = 0; i < count && i < objectList->items.size(); i++) {
                builder.items.push_back(objectList->items.at(i));
            }
        }
        result->returnedObject = boost::make_intrusive_ptr<ObjectList>(&builder);
    }
}

template <typename TList>
static boost::intrusive_ptr<TList> removeAtSingle(const TList& list, int64_t index) {
    if (index < 0 || static_cast<size_t>(index) >= list.items.size()) {
        throw Error(ErrorCode::kListIndexOutOfRange, "Index out of range.");
    }
    return boost::make_intrusive_ptr<TList>(list, index);
}

template <typename TList>
static boost::intrusive_ptr<TList> removeAtMultiple(const TList& list, const ValueList& indices) {
    std::vector<int64_t> sortedIndices{};
    for (const auto& index : indices.items) {
        sortedIndices.push_back(index.getInt64());
    }

    // Sort the indices in descending order.
    std::sort(sortedIndices.begin(), sortedIndices.end(), std::greater<int64_t>{});

    // Remove duplicate indices.
    sortedIndices.erase(std::unique(sortedIndices.begin(), sortedIndices.end()), sortedIndices.end());

    auto vec = list.items;

    for (auto index : sortedIndices) {
        if (index < 0 || static_cast<size_t>(index) >= list.items.size()) {
            throw Error(ErrorCode::kListIndexOutOfRange, "Index out of range.");
        }
        vec = vec.erase(static_cast<size_t>(index));
    }

    return boost::make_intrusive_ptr<TList>(vec);
}

// (input as List of T, start as Number, count as Number) as List of T
void systemCallListMid(const SystemCallInput& input, SystemCallResult* result) {
    // If the requested count is greater than the number of items in the list, don't throw an error, just return as many
    // items as there are.
    const auto countSigned = input.getValue(-1).getInt64();
    if (countSigned < 0) {
        throw Error(ErrorCode::kInvalidArgument, "Count must be non-negative");
    }
    auto count = static_cast<size_t>(countSigned);

    const auto startSigned = input.getValue(-2).getInt64();
    if (startSigned < 0) {
        throw Error(ErrorCode::kListIndexOutOfRange, "Start must be non-negative");
    }
    auto start = static_cast<size_t>(startSigned);

    const auto valueOrObject = valueOrObjectList(input.getObject(-1));
    const auto* valueList = valueOrObject.first;
    const auto* objectList = valueOrObject.second;

    if (valueList != nullptr) {
        if (start < 0 || start >= valueList->items.size()) {
            throw Error(ErrorCode::kListIndexOutOfRange, "Index out of range.");
        }
        ValueListBuilder builder{};
        auto takeCount = std::min(count, valueList->items.size() - start);
        for (size_t i = start; i < start + takeCount; i++) {
            builder.items.push_back(valueList->items.at(i));
        }
        result->returnedObject = boost::make_intrusive_ptr<ValueList>(&builder);
    } else {
        if (start < 0 || start >= objectList->items.size()) {
            throw Error(ErrorCode::kListIndexOutOfRange, "Index out of range.");
        }
        ObjectListBuilder builder{};
        auto takeCount = std::min(count, objectList->items.size() - start);
        for (size_t i = start; i < start + takeCount; i++) {
            builder.items.push_back(objectList->items.at(i));
        }
        result->returnedObject = boost::make_intrusive_ptr<ObjectList>(&builder);
    }
}

// (value as TObject, count as Number) as List of TObject
void systemCallListFillO(const SystemCallInput& input, SystemCallResult* result) {
    const auto& object = input.getObjectPtr(-1);

    const auto& countSigned = input.getValue(-1).getInt64();
    if (countSigned < 0) {
        throw Error(ErrorCode::kInvalidArgument, "Count must be non-negative.");
    }
    auto count = static_cast<size_t>(countSigned);

    ObjectListBuilder builder{};
    for (size_t i = 0; i < count; i++) {
        builder.items.push_back(object);
    }
    result->returnedObject = boost::make_intrusive_ptr<ObjectList>(&builder);
}

// (value as TValue, count as Number) as List of TValue
void systemCallListFillV(const SystemCallInput& input, SystemCallResult* result) {
    const auto& value = input.getValue(-2);

    const auto& countSigned = input.getValue(-1).getInt64();
    if (countSigned < 0) {
        throw Error(ErrorCode::kInvalidArgument, "Count must be non-negative.");
    }
    auto count = static_cast<size_t>(countSigned);

    ValueListBuilder builder{};
    for (size_t i = 0; i < count; i++) {
        builder.items.push_back(value);
    }
    result->returnedObject = boost::make_intrusive_ptr<ValueList>(&builder);
}

// (list as List of T) as T
void systemCallListFirst(const SystemCallInput& input, SystemCallResult* result) {
    listFirstOrLast(input, result, true);
}

// (list as List of T) as T
void systemCallListLast(const SystemCallInput& input, SystemCallResult* result) {
    listFirstOrLast(input, result, false);
}

// (input as List of T) as Number
void systemCallListLen(const SystemCallInput& input, SystemCallResult* result) {
    result->returnedValue.num = dynamic_cast<const ListBase&>(*input.getObject(-1)).size();
}

// (collection as List of T, index as Number) as List of T
void systemCallListRemoveAt1(const SystemCallInput& input, SystemCallResult* result) {
    const auto& listObject = input.getObject(-1);
    auto index = input.getValue(-1).getInt64();

    switch (listObject->getObjectType()) {
        case ObjectType::kObjectList:
            result->returnedObject = removeAtSingle<ObjectList>(*castObjectList(listObject), index);
            break;

        case ObjectType::kValueList:
            result->returnedObject = removeAtSingle<ValueList>(*castValueList(listObject), index);
            break;

        default:
            assert(false);
            break;
    }
}

// (collection as List of T, indices as List of Number) as List of T
void systemCallListRemoveAt2(const SystemCallInput& input, SystemCallResult* result) {
    const auto& listObject = input.getObject(-2);
    auto* indicesList = castValueList(input.getObject(-1));

    switch (listObject->getObjectType()) {
        case ObjectType::kObjectList:
            result->returnedObject = removeAtMultiple<ObjectList>(*castObjectList(listObject), *indicesList);
            break;

        case ObjectType::kValueList:
            result->returnedObject = removeAtMultiple<ValueList>(*castValueList(listObject), *indicesList);
            break;

        default:
            assert(false);
            break;
    }
}

// (input as List of T, count as Number) as List of T
void systemCallListSkip(const SystemCallInput& input, SystemCallResult* result) {
    listSkipOrTake(input, result, true);
}

// (input as List of T, count as Number) as List of T
void systemCallListTake(const SystemCallInput& input, SystemCallResult* result) {
    listSkipOrTake(input, result, false);
}

// (lhs as ObjectList, rhs as Object) as ObjectList
void systemCallObjectListAdd(const SystemCallInput& input, SystemCallResult* result) {
    const auto& objectList = *castObjectList(input.getObject(-2));
    const auto& object = input.getObjectPtr(-1);
    result->returnedObject = boost::make_intrusive_ptr<ObjectList>(objectList, true, objectList.size(), object);
}

// () as ObjectListBuilder
void systemCallObjectListBuilderNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_intrusive_ptr<ObjectListBuilder>();
}

// (builder as ObjectListBuilder, Object as Object)
void systemCallObjectListBuilderAdd(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto& builder = *castObjectListBuilder(input.getObject(-2));
    auto obj = input.getObjectPtr(-1);
    assert(obj->getObjectType() != ObjectType::kObjectListBuilder);
    assert(obj->getObjectType() != ObjectType::kValueListBuilder);
    builder.items.push_back(std::move(obj));
}

// (builder as ObjectListBuilder) as ObjectList
void systemCallObjectListBuilderEnd(const SystemCallInput& input, SystemCallResult* result) {
    auto& builder = *castObjectListBuilder(input.getObject(-1));
    result->returnedObject = boost::make_intrusive_ptr<ObjectList>(&builder);
}

// (lhs as ObjectList, rhs as ObjectList) as ObjectList
void systemCallObjectListConcat(const SystemCallInput& input, SystemCallResult* result) {
    const auto& lhs = *castObjectList(input.getObject(-2));
    const auto& rhs = *castObjectList(input.getObject(-1));
    ObjectListBuilder builder{ lhs.items.transient() };
    for (const auto& item : rhs.items) {
        builder.items.push_back(item);
    }
    result->returnedObject = boost::make_intrusive_ptr<ObjectList>(&builder);
}

// (input as ObjectList, index as Number) as Object
void systemCallObjectListGet(const SystemCallInput& input, SystemCallResult* result) {
    const auto& objectList = *castObjectList(input.getObject(-1));
    const auto& index = input.getValue(-1).getInt64();
    result->returnedObject = objectList.items.at(index);
    assert(result->returnedObject != nullptr);
}

// (input as ObjectList, item as Object) as ObjectList
void systemCallObjectListRemove(const SystemCallInput& input, SystemCallResult* result) {
    auto items = castObjectList(input.getObject(-1))->items;
    const auto& item = *input.getObject(-1);

    for (size_t i = items.size(); i > 0; i--) {
        if (items.at(i - 1)->equals(item)) {
            items = items.erase(i - 1);
        }
    }

    result->returnedObject = boost::make_intrusive_ptr<ObjectList>(std::move(items));
}

// (input as ObjectList, index as Number, value as Object) as ObjectList
void systemCallObjectListSet(const SystemCallInput& input, SystemCallResult* result) {
    const auto& objectList = *castObjectList(input.getObject(-2));
    const auto& index = static_cast<size_t>(input.getValue(-1).getInt64());
    const auto& element = input.getObjectPtr(-1);
    result->returnedObject = boost::make_intrusive_ptr<ObjectList>(objectList, /* insert */ false, index, element);
}

// (lhs as ValueList, rhs as Value) as ValueList
void systemCallValueListAdd(const SystemCallInput& input, SystemCallResult* result) {
    const auto& valueList = *castValueList(input.getObject(-1));
    const auto& value = input.getValue(-1);
    result->returnedObject = boost::make_intrusive_ptr<ValueList>(valueList, true, valueList.size(), value);
}

// () as ValueListBuilder
void systemCallValueListBuilderNew(const SystemCallInput& /*input*/, SystemCallResult* result) {
    result->returnedObject = boost::make_intrusive_ptr<ValueListBuilder>();
}

// (builder as ValueListBuilder, value as Value)
void systemCallValueListBuilderAdd(const SystemCallInput& input, SystemCallResult* /*result*/) {
    auto& builder = *castValueListBuilder(input.getObject(-1));
    builder.items.push_back(input.getValue(-1));
}

// (builder as ValueListBuilder) as ValueList
void systemCallValueListBuilderEnd(const SystemCallInput& input, SystemCallResult* result) {
    auto& builder = *castValueListBuilder(input.getObject(-1));
    result->returnedObject = boost::make_intrusive_ptr<ValueList>(&builder);
}

// (lhs as ValueList, rhs as ValueList) as ValueList
void systemCallValueListConcat(const SystemCallInput& input, SystemCallResult* result) {
    const auto& lhs = *castValueList(input.getObject(-2));
    const auto& rhs = *castValueList(input.getObject(-1));
    ValueListBuilder builder{ lhs.items.transient() };
    for (const auto& item : rhs.items) {
        builder.items.push_back(item);
    }
    result->returnedObject = boost::make_intrusive_ptr<ValueList>(&builder);
}

// (input as ValueList, index as Number) as Value
void systemCallValueListGet(const SystemCallInput& input, SystemCallResult* result) {
    const auto& valueList = *castValueList(input.getObject(-1));
    const auto& index = input.getValue(-1).getInt64();
    result->returnedValue = valueList.items.at(index);
}

// (input as ValueList, item as Value) as ValueList
void systemCallValueListRemove(const SystemCallInput& input, SystemCallResult* result) {
    auto items = castValueList(input.getObject(-1))->items;
    const auto& item = input.getValue(-1);

    for (size_t i = items.size(); i > 0; i--) {
        if (items.at(i - 1) == item) {
            items = items.erase(i - 1);
        }
    }

    result->returnedObject = boost::make_intrusive_ptr<ValueList>(std::move(items));
}

// (input as ValueList, index as Number, value as Value) as ValueList
void systemCallValueListSet(const SystemCallInput& input, SystemCallResult* result) {
    const auto& valueList = *castValueList(input.getObject(-1));
    const auto& index = static_cast<size_t>(input.getValue(-2).getInt64());
    const auto& value = input.getValue(-1);
    result->returnedObject = boost::make_intrusive_ptr<ValueList>(valueList, /* insert */ false, index, value);
}

}  // namespace vm
