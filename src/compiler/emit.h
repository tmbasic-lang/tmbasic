#pragma once

#include "../common.h"
#include "CompiledProgram.h"
#include "ast.h"
#include "vm/Procedure.h"

namespace compiler {

std::vector<uint8_t> emit(
    const ProcedureNode& procedureNode,
    int numLocalValues,
    int numLocalObjects,
    CompiledProgram* compiledProgram);

}  // namespace compiler
