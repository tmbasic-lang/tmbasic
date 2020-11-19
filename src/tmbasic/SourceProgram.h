#pragma once

#include "../common.h"
#include "shared/vm/ProgramMember.h"

namespace tmbasic {

enum class SourceMemberType { kProcedure, kGlobalVariable, kConstant, kType };

class SourceMember {
   public:
    SourceMemberType memberType;
    std::string displayName;

    std::string source;
    int selectionStart;
    int selectionEnd;

    vm::ProgramMember* compiledMember = nullptr;
    bool isCompiledMemberUpToDate = false;

    SourceMember(SourceMemberType memberType, std::string source, int selectionStart, int selectionEnd);

    void updateDisplayName();
};

class SourceProgram {
   public:
    std::vector<std::unique_ptr<SourceMember>> members;

    void save(std::string filePath) const;
};

}  // namespace tmbasic
