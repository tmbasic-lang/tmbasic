#include "../common.h"
#include "assemble.h"
#include "compiler/compileProgram.h"
#include "compiler/TargetPlatform.h"
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

static TargetPlatform getCurrentPlatform() {
    auto platform = string(getenv("TARGET_OS")) + "/" + getenv("ARCH");
    if (platform == "linux/i686") {
        return TargetPlatform::kLinuxX86;
    } else if (platform == "linux/x86_64") {
        return TargetPlatform::kLinuxX64;
    } else if (platform == "linux/arm32v7") {
        return TargetPlatform::kLinuxArm32;
    } else if (platform == "linux/arm64v8") {
        return TargetPlatform::kLinuxArm64;
    } else if (platform == "mac/x86_64") {
        return TargetPlatform::kMacX64;
    } else if (platform == "mac/arm64v8") {
        return TargetPlatform::kMacArm64;
    } else if (platform == "win/i686") {
        return TargetPlatform::kWinX86;
    } else if (platform == "win/x86_64") {
        return TargetPlatform::kWinX64;
    } else {
        throw runtime_error(string("Invalid platform: ") + platform);
    }
}

static vector<uint8_t> buildDummyBytecode(size_t length) {
    auto bytecode = vector<uint8_t>(length);
    for (size_t i = 0; i < length; i++) {
        bytecode.at(i) = i & 0xFF;
    }
    return bytecode;
}

static bool contains(const vector<uint8_t>& haystack, const vector<uint8_t>& needle) {
    auto firstByte = needle.at(0);
    auto secondByte = needle.at(1);
    auto verifySize = std::min<size_t>(100, needle.size());
    auto n = haystack.size() - needle.size();
    for (size_t i = 0; i < n; i++) {
        if (haystack[i] == firstByte && haystack[i + 1] == secondByte &&
            memcmp(&haystack[i], needle.data(), verifySize) == 0) {
            return true;
        }
    }

    return false;
}

static void run(string filenameWithoutExtension, compiler::CompiledProgram* program) {
    auto pcodeFile = readFile(filenameWithoutExtension + ".bas");

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
    auto compilerResult = compiler::compileProgram(sourceProgram, program);
    if (!compilerResult.isSuccess) {
        std::cerr << compilerResult.message << std::endl
                  << NAMEOF_ENUM(compilerResult.token.type) << " \"" << compilerResult.token.text << "\" ("
                  << compilerResult.token.lineIndex + 1 << ":" << compilerResult.token.columnIndex + 1 << ")"
                  << std::endl;
    }
    ASSERT_EQ(1, program->vmProgram.procedures.size());

    istringstream consoleInputStream(input);
    ostringstream consoleOutputStream;
    auto interpreter = make_unique<Interpreter>(&program->vmProgram, &consoleInputStream, &consoleOutputStream);
    interpreter->init(0);
    while (interpreter->run(10000)) {
    }
    auto actualOutput = consoleOutputStream.str();
    ASSERT_EQ(expectedOutput, actualOutput);
}

static void run(string filenameWithoutExtension) {
    compiler::CompiledProgram compiledProgram{};
    run(filenameWithoutExtension, &compiledProgram);
}

TEST(CompilerTest, BuildDummyBytecode) {
    auto bytecode = buildDummyBytecode(1000);
    ASSERT_EQ(0, bytecode.at(0));
    ASSERT_EQ(1, bytecode.at(1));
    ASSERT_EQ(2, bytecode.at(2));
    ASSERT_EQ(0, bytecode.at(256));
    ASSERT_EQ(1, bytecode.at(257));
}

TEST(CompilerTest, Contains) {
    auto haystack = vector<uint8_t>{ 1, 2, 3, 4, 5, 6 };
    auto needle1 = vector<uint8_t>{ 3, 4 };
    auto needle2 = vector<uint8_t>{ 4, 4 };

    ASSERT_TRUE(contains(haystack, needle1));
    ASSERT_FALSE(contains(haystack, needle2));
}

TEST(CompilerTest, MakeExeFile) {
    auto bytecode = buildDummyBytecode(10000);
    auto exe = makeExeFile(bytecode, getCurrentPlatform());
    ASSERT_TRUE(contains(exe, bytecode));
}

TEST(CompilerTest, EmptyMain) {
    run("EmptyMain");
}

TEST(CompilerTest, GlobalValue_Number) {
    compiler::CompiledProgram program{};
    run("GlobalValue_Number", &program);
    ASSERT_EQ(0, program.vmProgram.globalValues.at(0).getInt32());
}

TEST(CompilerTest, GlobalValue_String) {
    compiler::CompiledProgram program{};
    run("GlobalValue_String", &program);
    auto* o = program.vmProgram.globalObjects.at(0).get();
    ASSERT_NE(nullptr, o);
    ASSERT_EQ(vm::ObjectType::kString, o->getObjectType());
    auto str = dynamic_cast<vm::String*>(o)->toUtf8();
    ASSERT_EQ("test", str);
}
