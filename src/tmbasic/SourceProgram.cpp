#include "SourceProgram.h"

namespace tmbasic {

SourceMember::SourceMember(SourceMemberType memberType, std::string source, int selectionStart, int selectionEnd)
    : memberType(memberType), source(source), selectionStart(selectionStart), selectionEnd(selectionEnd) {
    updateDisplayName();
}

static bool isBlankOrComment(std::string str) {
    for (auto ch : str) {
        if (ch == '\'') {
            return true;
        } else if (ch != ' ' && ch != '\t') {
            return false;
        }
    }
    return true;
}

void SourceMember::updateDisplayName() {
    std::istringstream stream(source);
    std::string line;
    while (std::getline(stream, line)) {
        if (!isBlankOrComment(line)) {
            displayName = line;
            return;
        }
    }
    displayName = "Untitled";
}

static std::vector<const SourceMember*> sortMembers(const SourceProgram* program) {
    std::vector<const SourceMember*> sortedMembers;
    for (auto& x : program->members) {
        sortedMembers.push_back(x.get());
    }
    std::sort(sortedMembers.begin(), sortedMembers.end(), [](const auto* a, const auto* b) -> bool {
        if (a->memberType == b->memberType) {
            return a->displayName > b->displayName;
        } else {
            return a->memberType > b->memberType;
        }
    });
    return sortedMembers;
}

void SourceProgram::save(std::string filePath) const {
    auto stream = std::ofstream(filePath);

    for (auto* member : sortMembers(this)) {
        auto trimmedSource = boost::trim_copy(member->source);
        if (member->isCompiledMemberUpToDate && member->compiledMember) {
            // this code was compiled successfully so we know it's valid
            stream << trimmedSource << "\n\n";
        } else {
            // since we don't know that this is valid, use a #disabled block so this file will parse again in the face
            // of syntax errors in this block
            stream << "#disabled ";
            switch (member->memberType) {
                case SourceMemberType::kConstant:
                    stream << "const";
                    break;
                case SourceMemberType::kGlobalVariable:
                    stream << "dim";
                    break;
                case SourceMemberType::kProcedure:
                    stream << "procedure";
                    break;
                case SourceMemberType::kType:
                    stream << "type";
                    break;
                default:
                    assert(false);
            }
            boost::replace_all(trimmedSource, "#", "##");  // escape # symbol
            stream << "\n"
                   << trimmedSource << "\n"
                   << "#end"
                   << "\n\n";
        }
    }
}

}  // namespace tmbasic
