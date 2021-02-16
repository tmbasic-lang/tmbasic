#pragma once

#include "../common.h"
#include "vm/Procedure.h"
#include "compiler/ast.h"

namespace compiler {

std::vector<uint8_t> emit(const ProcedureNode& procedureNode);

}  // namespace compiler
