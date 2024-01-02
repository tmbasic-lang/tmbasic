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

static std::regex updateDisplayNameRegex("^[^ ]+ ([^( ]+)[ ]*(:?\\(.*)?$", std::regex::optimize);

void SourceMember::updateDisplayName() {
    std::istringstream stream(source);
    std::string line;
    while (std::getline(stream, line)) {
        if (!isBlankOrComment(line)) {
            displayName = line;

            std::smatch match;
            if (std::regex_search(displayName, match, updateDisplayNameRegex)) {
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
        members->push_back(std::make_unique<SourceMember>(currentMemberType, boost::trim_copy(s.str()) + "\n", 0, 0));
        currentBlock->clear();
    }
}

void SourceProgram::load(const std::string& filePath) {
    std::ostringstream s;
    std::ifstream file(filePath);
    if (!file) {
        throw std::system_error(errno, std::system_category());
    }
    std::string line;
    while (std::getline(file, line)) {
        s << line << "\n";
    }
    loadFromContent(s.str());
}

static void eatLinesUntilEnd(
    std::istringstream* stream,
    std::vector<std::string>* currentBlock,
    const std::string& endPhrase) {
    std::string line;
    while (std::getline(*stream, line)) {
        currentBlock->push_back(line);

        // The end phrase is case insensitive.
        auto lc = boost::to_lower_copy(line);

        if (lc.find(endPhrase) == 0) {
            return;
        }
    }
}

void SourceProgram::loadFromContent(const std::string& content) {
    // If this has Windows CRLF line endings, convert it to Unix LF line endings.
    if (content.find("\r\n") != std::string::npos) {
        loadFromContent(boost::replace_all_copy(content, "\r\n", "\n"));
        return;
    }

    std::istringstream file{ content };
    members.clear();

    std::vector<std::string> currentBlock;
    auto currentMemberType = SourceMemberType::kProcedure;
    std::string line;

    while (std::getline(file, line)) {
        currentBlock.push_back(line);

        // These keywords are case insensitive.
        auto lc = boost::to_lower_copy(line);

        // Detect the type of block by the starting keyword.
        // There can be comment lines above this.
        if (lc.find("design") == 0) {
            currentMemberType = SourceMemberType::kDesign;
            eatLinesUntilEnd(&file, &currentBlock, "end design");
            loadEndCurrentBlock(&currentBlock, &members, currentMemberType);
        } else if (lc.find("dim") == 0 || line.find("const") == 0) {
            currentMemberType = SourceMemberType::kGlobal;
            loadEndCurrentBlock(&currentBlock, &members, currentMemberType);
        } else if (lc.find("picture") == 0) {
            currentMemberType = SourceMemberType::kPicture;
            eatLinesUntilEnd(&file, &currentBlock, "end picture");
            loadEndCurrentBlock(&currentBlock, &members, currentMemberType);
        } else if (lc.find("sub") == 0) {
            currentMemberType = SourceMemberType::kProcedure;
            eatLinesUntilEnd(&file, &currentBlock, "end sub");
            loadEndCurrentBlock(&currentBlock, &members, currentMemberType);
        } else if (lc.find("function") == 0) {
            currentMemberType = SourceMemberType::kProcedure;
            eatLinesUntilEnd(&file, &currentBlock, "end function");
            loadEndCurrentBlock(&currentBlock, &members, currentMemberType);
        } else if (lc.find("type") == 0) {
            currentMemberType = SourceMemberType::kType;
            eatLinesUntilEnd(&file, &currentBlock, "end type");
            loadEndCurrentBlock(&currentBlock, &members, currentMemberType);
        }
    }

    // If there's anything in currentBlock, it's lines at the end that didn't include the start of a block.
    // We will just discard those lines.
    // Conveniently, when opening our own test programs, these discarded lines are the expected output that is invalid
    // TMBASIC code anyway.
}

void SourceProgram::save(const std::string& filePath) const {
    auto stream = std::ofstream(filePath);

    for (const auto* member : sortMembers(this)) {
        stream << boost::trim_copy(member->source) << "\n\n";
    }
}

void SourceProgram::forEachMember(SourceMemberType type, const std::function<void(const SourceMember&)>& callback)
    const {
    for (const auto& member : members) {
        if (member->memberType == type) {
            callback(*member);
        }
    }
}

void SourceProgram::forEachMemberIndex(
    SourceMemberType type,
    const std::function<void(const SourceMember&, size_t)>& callback) const {
    for (size_t i = 0; i < members.size(); i++) {
        auto& member = *members.at(i);
        if (member.memberType == type) {
            callback(member, i);
        }
    }
}

}  // namespace compiler
