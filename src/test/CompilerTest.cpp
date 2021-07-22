#include "../common.h"
#include "assemble.h"
#include "compiler/CompilerException.h"
#include "compiler/TargetPlatform.h"
#include "compiler/compileProgram.h"
#include "compiler/makeExeFile.h"
#include "gtest/gtest.h"
#include "helpers.h"
#include "vm/Interpreter.h"
#include "vm/Procedure.h"
#include "vm/Program.h"
#include "vm/String.h"

using compiler::makeExeFile;
using compiler::TargetPlatform;
using std::istringstream;
using std::make_unique;
using std::ostringstream;
using std::runtime_error;
using std::string;
using std::vector;
using vm::Interpreter;

static void run(string filenameWithoutExtension, compiler::CompiledProgram* program) {
    auto pcodeFile = readFile(string("CompilerTest/") + filenameWithoutExtension + ".bas");

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

    compiler::SourceProgram sourceProgram;
    sourceProgram.loadFromContent(source);
    try {
        compiler::compileProgram(sourceProgram, program);
    } catch (compiler::CompilerException& ex) {
        std::cerr << ex.message << std::endl
                  << NAMEOF_ENUM(ex.token.type) << " \"" << ex.token.text << "\" (" << ex.token.lineIndex + 1 << ":"
                  << ex.token.columnIndex + 1 << ")" << std::endl;
    }

    istringstream consoleInputStream(input);
    ostringstream consoleOutputStream;
    auto interpreter = make_unique<Interpreter>(&program->vmProgram, &consoleInputStream, &consoleOutputStream);
    interpreter->init(program->vmProgram.startupProcedureIndex);
    while (interpreter->run(10000)) {
    }
    auto actualOutput = consoleOutputStream.str();
    ASSERT_EQ(expectedOutput, actualOutput);
}

static void run(string filenameWithoutExtension) {
    compiler::CompiledProgram compiledProgram{};
    run(filenameWithoutExtension, &compiledProgram);
}

TEST(CompilerTest, GlobalValue_Number_CheckValue) {
    compiler::CompiledProgram program{};
    run("global_value_number", &program);
    ASSERT_EQ(0, program.vmProgram.globalValues.at(0).getInt32());
}

TEST(CompilerTest, GlobalValue_String_CheckValue) {
    compiler::CompiledProgram program{};
    run("global_value_string", &program);
    auto* o = program.vmProgram.globalObjects.at(0).get();
    ASSERT_NE(nullptr, o);
    ASSERT_EQ(vm::ObjectType::kString, o->getObjectType());
    auto str = dynamic_cast<vm::String*>(o)->toUtf8();
    ASSERT_EQ("test", str);
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define COMPILER_TEST(x) \
    TEST(CompilerTest, x) { run(#x); }

// Regenerate with:
/*
ls src/test/programs/CompilerTest | grep "\.bas$" | xargs -n 1 basename | sed "s/\.bas//g" | \
awk '{ print "COMPILER_TEST(" $1 ")" }'
*/

COMPILER_TEST(Chr)
COMPILER_TEST(boolean_literal)
COMPILER_TEST(call_function_one_arg)
COMPILER_TEST(call_function_zero_arg)
COMPILER_TEST(call_sub)
COMPILER_TEST(empty_main)
COMPILER_TEST(for_loop)
COMPILER_TEST(global_value_number)
COMPILER_TEST(global_value_string)
COMPILER_TEST(hello_world)
COMPILER_TEST(if_false)
COMPILER_TEST(input_string)
COMPILER_TEST(list_index)
COMPILER_TEST(main_not_first_in_program)
COMPILER_TEST(number_comparison)
COMPILER_TEST(print)
COMPILER_TEST(print_number)
COMPILER_TEST(variable_string)
COMPILER_TEST(while_loop)
