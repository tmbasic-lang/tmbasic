#pragma once

#include "../common.h"
#include "vm/Program.h"

std::unique_ptr<vm::Program> assemble(std::istream* input);
