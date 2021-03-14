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

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INTERPRETER_TEST(x) \
    TEST(InterpreterTest, x) { run(#x); }

// Regenerate with:
/*
ls src/test/programs/ | grep "\.pcode$" | xargs -n 1 basename | sed "s/\.pcode//g" | \
awk '{ print "INTERPRETER_TEST(" $1 ")" }'
*/

INTERPRETER_TEST(AddFunction)
INTERPRETER_TEST(AvailableLocales)
INTERPRETER_TEST(AvailableTimeZones)
INTERPRETER_TEST(DateFromParts)
INTERPRETER_TEST(DateTimeFromParts)
INTERPRETER_TEST(Errors)
INTERPRETER_TEST(ExitOnly)
INTERPRETER_TEST(HasValueO)
INTERPRETER_TEST(HasValueV)
INTERPRETER_TEST(HelloWorld)
INTERPRETER_TEST(Julia)
INTERPRETER_TEST(NumberToString)
INTERPRETER_TEST(ObjectList)
INTERPRETER_TEST(ObjectToObjectMap)
INTERPRETER_TEST(ObjectToValueMap)
INTERPRETER_TEST(Record)
INTERPRETER_TEST(SimpleMath)
INTERPRETER_TEST(StringAsc)
INTERPRETER_TEST(StringCharacters1_Empty)
INTERPRETER_TEST(StringCharacters1_English)
INTERPRETER_TEST(StringCharacters2_English)
INTERPRETER_TEST(StringCharacters2_InvalidLocale)
INTERPRETER_TEST(StringChr)
INTERPRETER_TEST(StringChrCombiningDiacritic)
INTERPRETER_TEST(StringIndexOf)
INTERPRETER_TEST(StringInputLine)
INTERPRETER_TEST(StringLen)
INTERPRETER_TEST(StringMid)
INTERPRETER_TEST(TimeSpanNew)
INTERPRETER_TEST(TimeSpanTotal)
INTERPRETER_TEST(UtcOffset)
INTERPRETER_TEST(ValueList)
INTERPRETER_TEST(ValueO)
INTERPRETER_TEST(ValueToObjectMap)
INTERPRETER_TEST(ValueToValueMap)
INTERPRETER_TEST(ValueV)
