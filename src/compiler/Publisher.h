#pragma once

#include "CompiledProgram.h"
#include "TargetPlatform.h"

namespace compiler {

class Publisher {
   public:
    Publisher(const CompiledProgram& compiledProgram, const std::string& basFilePath);
    std::string getPublishDir() const;
    std::string publish(TargetPlatform platform);

   private:
    static std::string getPublishDir(const std::string& basFilePath);

    const std::string _filename;
    const std::string _publishDir;
    const std::vector<uint8_t> _pcode{};
};

}  // namespace compiler
