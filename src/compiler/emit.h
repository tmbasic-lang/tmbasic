#pragma once

#include "../common.h"
#include "vm/Procedure.h"
#include "compiler/ast.h"

namespace compiler {

std::vector<uint8_t> emit(const ProcedureNode& procedureNode, int numLocalValues, int numLocalObjects);

}  // namespace compiler
