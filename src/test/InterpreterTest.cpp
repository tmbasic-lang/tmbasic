#include "../common.h"
#include "assemble.h"
#include "gtest/gtest.h"
#include "helpers.h"
#include "shared/vm/Interpreter.h"
#include "shared/vm/Procedure.h"
#include "shared/vm/ProcedureArtifact.h"
#include "shared/vm/Program.h"

using std::istringstream;
using std::make_unique;
using std::move;
using std::string;
using vm::Interpreter;
using vm::Procedure;
using vm::ProcedureArtifact;
using vm::Program;

static void run(string filenameWithoutExtension) {
    auto source = readFile(filenameWithoutExtension + ".asm");
    auto output = readFile(filenameWithoutExtension + ".txt");
    istringstream sourceStream(source);
    auto artifact = make_unique<ProcedureArtifact>();
    artifact->instructions = assemble(&sourceStream);
    auto procedure = make_unique<Procedure>();
    procedure->artifact = move(artifact);
    auto program = make_unique<Program>();
    program->procedures.push_back(move(procedure));
    auto interpreter = make_unique<Interpreter>(*program);
    interpreter->init(0);
    interpreter->run(10000);
}

TEST(InterpreterTest, ExitOnly) {
    run("ExitOnly");
}
