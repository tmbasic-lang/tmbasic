#include "../common.h"
#include "assemble.h"
#include "gtest/gtest.h"
#include "helpers.h"
#include "vm/Interpreter.h"
#include "vm/Procedure.h"
#include "vm/Program.h"

using std::istringstream;
using std::make_unique;
using std::move;
using std::ostringstream;
using std::string;
using vm::Interpreter;
using vm::Procedure;
using vm::Program;

static void run(string filenameWithoutExtension) {
    auto pcodeFile = readFile(filenameWithoutExtension + ".pcode");

    string inputSentinel = "--input--\n";
    auto inputStart = pcodeFile.find(inputSentinel);

    string outputSentinel = "--output--\n";
    auto outputStart = pcodeFile.find(outputSentinel);

    string input;
    if (inputStart != string::npos) {
        if (outputStart == string::npos) {
            input = pcodeFile.substr(inputStart + inputSentinel.size());
        } else {
            input =
                pcodeFile.substr(inputStart + inputSentinel.size(), outputStart - inputStart - inputSentinel.size());
        }
    }

    string expectedOutput;
    if (outputStart != string::npos) {
        expectedOutput = pcodeFile.substr(outputStart + outputSentinel.size());
    }

    string source;
    if (inputStart != string::npos) {
        source = pcodeFile.substr(0, inputStart);
    } else if (outputStart != string::npos) {
        source = pcodeFile.substr(0, outputStart);
    } else {
        source = pcodeFile;
    }

    istringstream sourceStream(source);
    auto program = assemble(&sourceStream);
    istringstream consoleInputStream(input);
    ostringstream consoleOutputStream;
    auto interpreter = make_unique<Interpreter>(program.get(), &consoleInputStream, &consoleOutputStream);
    interpreter->init(0);
    while (interpreter->run(10000)) {
    }
    auto actualOutput = consoleOutputStream.str();
    ASSERT_EQ(expectedOutput, actualOutput);
}

// Regenerate with:
/*
ls src/test/programs/ | grep "\.pcode$" | xargs -n 1 basename | sed "s/\.pcode//g" | \
awk '{ print "TEST(InterpreterTest, " $1 ") { run(\"" $1 "\"); }" }'
*/

TEST(InterpreterTest, AddFunction) {
    run("AddFunction");
}
TEST(InterpreterTest, AvailableLocales) {
    run("AvailableLocales");
}
TEST(InterpreterTest, Errors) {
    run("Errors");
}
TEST(InterpreterTest, ExitOnly) {
    run("ExitOnly");
}
TEST(InterpreterTest, HasValueO) {
    run("HasValueO");
}
TEST(InterpreterTest, HasValueV) {
    run("HasValueV");
}
TEST(InterpreterTest, HelloWorld) {
    run("HelloWorld");
}
TEST(InterpreterTest, Julia) {
    run("Julia");
}
TEST(InterpreterTest, NumberToString) {
    run("NumberToString");
}
TEST(InterpreterTest, ObjectList) {
    run("ObjectList");
}
TEST(InterpreterTest, ObjectToObjectMap) {
    run("ObjectToObjectMap");
}
TEST(InterpreterTest, ObjectToValueMap) {
    run("ObjectToValueMap");
}
TEST(InterpreterTest, Record) {
    run("Record");
}
TEST(InterpreterTest, SimpleMath) {
    run("SimpleMath");
}
TEST(InterpreterTest, StringAsc) {
    run("StringAsc");
}
TEST(InterpreterTest, StringCharacters1_Empty) {
    run("StringCharacters1_Empty");
}
TEST(InterpreterTest, StringCharacters1_English) {
    run("StringCharacters1_English");
}
TEST(InterpreterTest, StringCharacters2_English) {
    run("StringCharacters2_English");
}
TEST(InterpreterTest, StringCharacters2_InvalidLocale) {
    run("StringCharacters2_InvalidLocale");
}
TEST(InterpreterTest, StringChr) {
    run("StringChr");
}
TEST(InterpreterTest, StringChrCombiningDiacritic) {
    run("StringChrCombiningDiacritic");
}
TEST(InterpreterTest, StringIndexOf) {
    run("StringIndexOf");
}
TEST(InterpreterTest, StringInputLine) {
    run("StringInputLine");
}
TEST(InterpreterTest, StringLen) {
    run("StringLen");
}
TEST(InterpreterTest, StringMid) {
    run("StringMid");
}
TEST(InterpreterTest, ValueList) {
    run("ValueList");
}
TEST(InterpreterTest, ValueO) {
    run("ValueO");
}
TEST(InterpreterTest, ValueToObjectMap) {
    run("ValueToObjectMap");
}
TEST(InterpreterTest, ValueToValueMap) {
    run("ValueToValueMap");
}
TEST(InterpreterTest, ValueV) {
    run("ValueV");
}
