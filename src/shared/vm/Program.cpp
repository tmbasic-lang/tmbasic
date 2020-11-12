#include "Program.h"

namespace vm {

void Program::save(std::string filePath) const {
    auto sortedProcedures = std::vector<Procedure*>();
    for (auto& x : procedures) {
        sortedProcedures.push_back(x.get());
    }
    std::sort(sortedProcedures.begin(), sortedProcedures.end(), [](const Procedure* a, const Procedure* b) -> bool {
        return a->name > b->name;
    });

    auto sortedGlobalVariables = std::vector<GlobalVariable*>();
    for (auto& x : globalVariables) {
        sortedGlobalVariables.push_back(x.get());
    }
    std::sort(
        sortedGlobalVariables.begin(), sortedGlobalVariables.end(),
        [](const GlobalVariable* a, const GlobalVariable* b) -> bool { return a->lowercaseName > b->lowercaseName; });

    auto stream = std::ofstream(filePath, ios::out);

    for (auto* procedure : sortedProcedures) {
        assert(procedure->source.has_value());
        stream << *procedure->source << "\n"
               << "\n";
    }

    for (auto* globalVariable : sortedGlobalVariables) {
        assert(globalVariable->source.has_value());
        stream << *globalVariable->source << "\n"
               << "\n";
    }
}

}  // namespace vm
