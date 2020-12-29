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
using std::ostringstream;
using std::string;
using vm::Interpreter;
using vm::Procedure;
using vm::ProcedureArtifact;
using vm::Program;

static void run(string filenameWithoutExtension) {
    auto source = readFile(filenameWithoutExtension + ".asm");
    auto expectedOutput = readFile(filenameWithoutExtension + ".txt");
    istringstream sourceStream(source);
    auto program = assemble(&sourceStream);
    istringstream consoleInputStream("");
    ostringstream consoleOutputStream;
    auto interpreter = make_unique<Interpreter>(*program, &consoleInputStream, &consoleOutputStream);
    interpreter->init(0);
    while (interpreter->run(10000)) {
    }
    auto actualOutput = consoleOutputStream.str();
    ASSERT_EQ(expectedOutput, actualOutput);
}

TEST(InterpreterTest, ExitOnly) {
    run("ExitOnly");
}

TEST(InterpreterTest, HelloWorld) {
    run("HelloWorld");
}

TEST(InterpreterTest, SimpleMath) {
    run("SimpleMath");
}

TEST(InterpreterTest, AddFunction) {
    run("AddFunction");
}

TEST(InterpreterTest, Julia) {
    run("Julia");
}
