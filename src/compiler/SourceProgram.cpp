#include "SourceProgram.h"

namespace compiler {

SourceMember::SourceMember(SourceMemberType memberType, std::string source, int selectionStart, int selectionEnd)
    : memberType(memberType), source(std::move(source)), selectionStart(selectionStart), selectionEnd(selectionEnd) {
    updateDisplayName();
}

static bool isBlankOrComment(const std::string& str) {
    for (auto ch : str) {
        if (ch == '\'') {
            return true;
        }
        if (ch != ' ' && ch != '\t') {
            return false;
        }
    }
    return true;
}

void SourceMember::setSource(std::string newSource) {
    isCompiledMemberUpToDate = false;
    source = std::move(newSource);
    updateDisplayName();
}

void SourceMember::updateDisplayName() {
    std::istringstream stream(source);
    std::string line;
    while (std::getline(stream, line)) {
        if (!isBlankOrComment(line)) {
            displayName = line;

            std::regex r("^[^ ]+ ([^( ]+)[ ]*(:?\\(.*)?$");
            std::smatch match;
            if (std::regex_search(displayName, match, r)) {
                identifier = match[1].str();
            } else {
                identifier = displayName;
            }

            return;
        }
    }

    displayName = "Untitled";
    identifier = "?";
}

static void insertSortedMembersOfType(
    const SourceProgram* program,
    SourceMemberType type,
    std::vector<const SourceMember*>* result) {
    std::vector<const SourceMember*> sortedMembers;
    for (const auto& x : program->members) {
        if (x->memberType == type) {
            sortedMembers.push_back(x.get());
        }
    }
    std::sort(sortedMembers.begin(), sortedMembers.end(), [](const auto* a, const auto* b) -> bool {
        if (a->memberType == b->memberType) {
            return a->displayName > b->displayName;
        }
        return a->memberType > b->memberType;
    });
    result->insert(result->end(), sortedMembers.begin(), sortedMembers.end());
}

static std::vector<const SourceMember*> sortMembers(const SourceProgram* program) {
    std::vector<const SourceMember*> result;
    insertSortedMembersOfType(program, SourceMemberType::kType, &result);
    insertSortedMembersOfType(program, SourceMemberType::kGlobal, &result);
    insertSortedMembersOfType(program, SourceMemberType::kProcedure, &result);
    insertSortedMembersOfType(program, SourceMemberType::kDesign, &result);
    insertSortedMembersOfType(program, SourceMemberType::kPicture, &result);
    return result;
}

static void loadEndCurrentBlock(
    std::vector<std::string>* currentBlock,
    std::vector<std::unique_ptr<SourceMember>>* members,
    SourceMemberType currentMemberType) {
    if (!currentBlock->empty()) {
        std::ostringstream s;
        for (auto& line : *currentBlock) {
            s << line << "\n";
        }
        members->push_back(std::make_unique<SourceMember>(
            currentMemberType, boost::trim_copy(boost::replace_all_copy(s.str(), "##", "#")) + "\n", 0, 0));
        currentBlock->clear();
    }
}

void SourceProgram::load(const std::string& filePath) {
    members.clear();

    std::ifstream file(filePath);
    if (!file) {
        throw std::system_error(errno, std::system_category());
    }

    std::vector<std::string> currentBlock;
    auto currentMemberType = SourceMemberType::kProcedure;
    std::string line;

    while (std::getline(file, line)) {
        auto isBlockDone = false;
        SourceMemberType memberType;  // set this when setting isBlockDone=true

        if (line.find("#design") == 0) {
            memberType = SourceMemberType::kDesign;
            isBlockDone = true;
        } else if (line.find("#global") == 0) {
            memberType = SourceMemberType::kGlobal;
            isBlockDone = true;
        } else if (line.find("#picture") == 0) {
            memberType = SourceMemberType::kPicture;
            isBlockDone = true;
        } else if (line.find("#procedure") == 0) {
            memberType = SourceMemberType::kProcedure;
            isBlockDone = true;
        } else if (line.find("#type") == 0) {
            memberType = SourceMemberType::kType;
            isBlockDone = true;
        }

        if (isBlockDone) {
            loadEndCurrentBlock(&currentBlock, &members, currentMemberType);
            currentMemberType = memberType;
        } else {
            currentBlock.push_back(line);
        }
    }

    loadEndCurrentBlock(&currentBlock, &members, currentMemberType);
}

void SourceProgram::save(const std::string& filePath) const {
    auto stream = std::ofstream(filePath);

    for (const auto* member : sortMembers(this)) {
        switch (member->memberType) {
            case SourceMemberType::kDesign:
                stream << "#design";
                break;
            case SourceMemberType::kGlobal:
                stream << "#global";
                break;
            case SourceMemberType::kPicture:
                stream << "#picture";
                break;
            case SourceMemberType::kProcedure:
                stream << "#procedure";
                break;
            case SourceMemberType::kType:
                stream << "#type";
                break;
            default:
                assert(false);
                break;
        }
        stream << "\n" << boost::trim_copy(boost::replace_all_copy(member->source, "#", "##")) << "\n\n";
    }
}

}  // namespace compiler
