#include "compileProgram.h"
#include "CompilerException.h"
#include "compileGlobals.h"
#include "compileProcedures.h"
#include "compileTypes.h"

namespace compiler {

void compileProgram(const SourceProgram& sourceProgram, CompiledProgram* compiledProgram) {
    compileTypes(sourceProgram, compiledProgram);
    compileGlobals(sourceProgram, compiledProgram);
    compileProcedures(sourceProgram, compiledProgram);
}

}  // namespace compiler
