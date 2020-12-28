#pragma once

#include "../common.h"
#include "shared/vm/Program.h"

std::unique_ptr<vm::Program> assemble(std::istream* input);
