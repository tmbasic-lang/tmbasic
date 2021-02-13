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
    auto pcodeFile = readFile(filenameWithoutExtension + ".pcode");

    std::string inputSentinel = "--input--\n";
    auto inputStart = pcodeFile.find(inputSentinel);

    std::string outputSentinel = "--output--\n";
    auto outputStart = pcodeFile.find(outputSentinel);

    std::string input;
    if (inputStart != std::string::npos) {
        if (outputStart == std::string::npos) {
            input = pcodeFile.substr(inputStart + inputSentinel.size());
        } else {
            input =
                pcodeFile.substr(inputStart + inputSentinel.size(), outputStart - inputStart - inputSentinel.size());
        }
    }

    std::string expectedOutput;
    if (outputStart != std::string::npos) {
        expectedOutput = pcodeFile.substr(outputStart + outputSentinel.size());
    }

    std::string source;
    if (inputStart != std::string::npos) {
        source = pcodeFile.substr(0, inputStart);
    } else if (outputStart != std::string::npos) {
        source = pcodeFile.substr(0, outputStart);
    } else {
        source = pcodeFile;
    }

    istringstream sourceStream(source);
    auto program = assemble(&sourceStream);
    istringstream consoleInputStream(input);
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

TEST(InterpreterTest, Errors) {
    run("Errors");
}

TEST(InterpreterTest, Record) {
    run("Record");
}

TEST(InterpreterTest, ValueList) {
    run("ValueList");
}

TEST(InterpreterTest, ObjectList) {
    run("ObjectList");
}

TEST(InterpreterTest, ValueToValueMap) {
    run("ValueToValueMap");
}

TEST(InterpreterTest, ValueToObjectMap) {
    run("ValueToObjectMap");
}

TEST(InterpreterTest, ObjectToValueMap) {
    run("ObjectToValueMap");
}

TEST(InterpreterTest, ObjectToObjectMap) {
    run("ObjectToObjectMap");
}

TEST(InterpreterTest, StringMid) {
    run("StringMid");
}

TEST(InterpreterTest, StringIndexOf) {
    run("StringIndexOf");
}

TEST(InterpreterTest, StringChr) {
    run("StringChr");
}

TEST(InterpreterTest, StringChrCombiningDiacritic) {
    run("StringChrCombiningDiacritic");
}

TEST(InterpreterTest, StringAsc) {
    run("StringAsc");
}

TEST(InterpreterTest, StringInputLine) {
    run("StringInputLine");
}

TEST(InterpreterTest, NumberToString) {
    run("NumberToString");
}

TEST(InterpreterTest, AvailableLocales) {
    run("AvailableLocales");
}

TEST(InterpreterTest, StringCharacters1_English) {
    run("StringCharacters1_English");
}

TEST(InterpreterTest, StringCharacters1_Empty) {
    run("StringCharacters1_Empty");
}

TEST(InterpreterTest, StringCharacters2_English) {
    run("StringCharacters2_English");
}

TEST(InterpreterTest, StringCharacters2_InvalidLocale) {
    run("StringCharacters2_InvalidLocale");
}

TEST(InterpreterTest, StringLen) {
    run("StringLen");
}

TEST(InterpreterTest, HasValueV) {
    run("HasValueV");
}

TEST(InterpreterTest, HasValueO) {
    run("HasValueO");
}
