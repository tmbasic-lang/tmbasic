#pragma once

#include "../common.h"
#include "CompiledProgram.h"
#include "ast.h"

namespace compiler {

std::vector<uint8_t> emit(
    const ProcedureNode& procedureNode,
    int numLocalValues,
    int numLocalObjects,
    CompiledProgram* compiledProgram);

}  // namespace compiler
