#include "BuiltInRecordTypesList.h"

namespace compiler {

class BuiltInRecordTypesList {
    using FieldList = std::vector<boost::local_shared_ptr<ParameterNode>>;

   public:
    BuiltInRecordTypesList() {
        auto number = boost::make_local_shared<TypeNode>(Kind::kNumber, Token{});

        types["rectangle"] = {
            ValueField(0, "Left", number),
            ValueField(1, "Top", number),
            ValueField(2, "Width", number),
            ValueField(3, "Height", number),
        };

        types["color"] = {
            ValueField(0, "Red", number),
            ValueField(1, "Green", number),
            ValueField(2, "Blue", number),
        };
    }

    std::unordered_map<std::string, FieldList> types{};

   private:
    static boost::local_shared_ptr<ParameterNode> ValueField(
        int index,
        std::string name,
        const boost::local_shared_ptr<TypeNode>& type) {
        auto param = boost::make_local_shared<ParameterNode>(std::move(name), type, Token{});
        param->fieldValueIndex = index;
        return param;
    }

    static boost::local_shared_ptr<ParameterNode> ObjectField(
        int index,
        std::string name,
        const boost::local_shared_ptr<TypeNode>& type) {
        auto param = boost::make_local_shared<ParameterNode>(std::move(name), type, Token{});
        param->fieldObjectIndex = index;
        return param;
    }
};

static const BuiltInRecordTypesList builtInRecordTypesList;

bool findBuiltInRecordType(
    const std::string& lowercaseName,
    std::vector<boost::local_shared_ptr<ParameterNode>>* outFieldList) {
    auto it = builtInRecordTypesList.types.find(lowercaseName);
    if (it != builtInRecordTypesList.types.end()) {
        outFieldList->insert(outFieldList->end(), it->second.begin(), it->second.end());
        return true;
    }
    return false;
}

}  // namespace compiler
