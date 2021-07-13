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
    auto pcodeFile = readFile(string("InterpreterTest/") + filenameWithoutExtension + ".pcode");

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
    Interpreter interpreter{ program.get(), &consoleInputStream, &consoleOutputStream };
    interpreter.init(program->startupProcedureIndex);
    while (interpreter.run(10000)) {
    }
    auto actualOutput = consoleOutputStream.str();
    ASSERT_EQ(expectedOutput, actualOutput);
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INTERPRETER_TEST(x) \
    TEST(InterpreterTest, x) { run(#x); }

// Regenerate with:
/*
ls src/test/programs/InterpreterTest | grep "\.pcode$" | xargs -n 1 basename | sed "s/\.pcode//g" | \
awk '{ print "INTERPRETER_TEST(" $1 ")" }'
*/

INTERPRETER_TEST(AddFunction)
INTERPRETER_TEST(AvailableLocales)
INTERPRETER_TEST(AvailableTimeZones)
INTERPRETER_TEST(DateFromParts)
INTERPRETER_TEST(DateTimeFromParts)
INTERPRETER_TEST(DateTimeOffsetFromParts)
INTERPRETER_TEST(Errors)
INTERPRETER_TEST(ExitOnly)
INTERPRETER_TEST(HasValueO)
INTERPRETER_TEST(HasValueV)
INTERPRETER_TEST(HelloWorld)
INTERPRETER_TEST(NumberToString)
