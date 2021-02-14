#pragma once

#include "../common.h"
#include "shared/vm/Procedure.h"
#include "shared/basic/ast.h"

namespace compiler {

std::vector<uint8_t> emit(const basic::ProcedureNode& procedureNode);

}  // namespace compiler
