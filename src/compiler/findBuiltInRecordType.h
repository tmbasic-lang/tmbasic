#pragma once

#include "../common.h"
#include "ast.h"

namespace compiler {

bool findBuiltInRecordType(
    const std::string& lowercaseName,
    std::vector<boost::local_shared_ptr<ParameterNode>>* outFieldList);

}  // namespace compiler
