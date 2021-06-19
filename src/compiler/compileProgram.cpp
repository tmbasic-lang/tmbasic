#include "compileProgram.h"
#include "compileGlobal.h"
#include "compileProcedure.h"

namespace compiler {

CompilerResult compileProgram(const SourceProgram& sourceProgram, CompiledProgram* compiledProgram) {
    for (const auto& member : sourceProgram.members) {
        if (member->memberType == SourceMemberType::kGlobal) {
            auto result = compileGlobal(*member, compiledProgram);
            if (!result.isSuccess) {
                return result;
            }
        }
    }
    for (const auto& member : sourceProgram.members) {
        if (member->memberType == SourceMemberType::kProcedure) {
            auto result = compileProcedure(*member, compiledProgram);
            if (!result.isSuccess) {
                return result;
            }
        }
    }
    return CompilerResult::success();
}

}  // namespace compiler
