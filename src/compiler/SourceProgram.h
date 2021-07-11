#pragma once

#include "../common.h"

namespace compiler {

enum class SourceMemberType { kProcedure, kGlobal, kType, kDesign, kPicture };

class SourceMember {
   public:
    SourceMemberType memberType;
    std::string displayName;
    std::string identifier;

    std::string source;
    int selectionStart;
    int selectionEnd;

    bool isCompiledMemberUpToDate = false;

    SourceMember(SourceMemberType memberType, std::string source, int selectionStart, int selectionEnd);
    void setSource(std::string newSource);

   private:
    void updateDisplayName();
};

class SourceProgram {
   public:
    std::vector<std::unique_ptr<SourceMember>> members;

    void loadFromContent(const std::string& content);
    void load(const std::string& filePath);
    void save(const std::string& filePath) const;
    void forEachMember(SourceMemberType type, std::function<void(const SourceMember&)> callback) const;
    void forEachMemberIndex(SourceMemberType type, std::function<void(const SourceMember&, size_t)> callback) const;
};

}  // namespace compiler
