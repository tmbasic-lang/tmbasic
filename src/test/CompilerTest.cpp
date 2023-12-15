#include "../common.h"
#include "compiler/CompilerException.h"
#include "compiler/TargetPlatform.h"
#include "compiler/compileProgram.h"
#include "gtest/gtest.h"
#include "helpers.h"
#include "vm/Interpreter.h"
#include "vm/Procedure.h"
#include "vm/Program.h"
#include "vm/String.h"
#include "vm/date.h"

using compiler::TargetPlatform;
using std::istringstream;
using std::make_unique;
using std::ostringstream;
using std::runtime_error;
using std::string;
using std::vector;
using vm::Interpreter;

static void runCode(const string& basFile) {
    vm::initializeTzdb();

    compiler::CompiledProgram program{};

    string inputSentinel = "--input--\n";
    auto inputStart = basFile.find(inputSentinel);

    string outputSentinel = "--output--\n";
    auto outputStart = basFile.find(outputSentinel);

    string input;
    if (inputStart != string::npos) {
        if (outputStart == string::npos) {
            input = basFile.substr(inputStart + inputSentinel.size());
        } else {
            input = basFile.substr(inputStart + inputSentinel.size(), outputStart - inputStart - inputSentinel.size());
        }
    }

    string expectedOutput;
    if (outputStart != string::npos) {
        expectedOutput = basFile.substr(outputStart + outputSentinel.size());
    }

    string source;
    if (inputStart != string::npos) {
        source = basFile.substr(0, inputStart);
    } else if (outputStart != string::npos) {
        source = basFile.substr(0, outputStart);
    } else {
        source = basFile;
    }

    istringstream consoleInputStream{ input };
    ostringstream consoleOutputStream{};

    compiler::SourceProgram sourceProgram{};
    sourceProgram.loadFromContent(source);
    bool compileSuccess = false;
    try {
        compiler::compileProgram(sourceProgram, &program);
        compileSuccess = true;
    } catch (compiler::CompilerException& ex) {
        consoleOutputStream << "Compiler error\n"
                            << NAMEOF_ENUM(ex.code) << "\n"
                            << (ex.token.sourceMember == nullptr ? "token.sourceMember is null!"
                                                                 : ex.token.sourceMember->identifier)
                            << "\n"
                            << (ex.token.lineIndex + 1) << ":" << (ex.token.columnIndex + 1) << "\n";
    }

    if (compileSuccess) {
        auto interpreter = make_unique<Interpreter>(&program.vmProgram, &consoleInputStream, &consoleOutputStream);
        interpreter->init(program.vmProgram.startupProcedureIndex);
        while (interpreter->run(10000)) {
        }

        auto error = interpreter->getError();
        if (error.has_value()) {
            consoleOutputStream << "Error" << std::endl
                                << error->code.getString() << std::endl
                                << error->message << std::endl;
        }
    }

    auto actualOutput = consoleOutputStream.str();
    ASSERT_EQ(expectedOutput, actualOutput);
}

static void run(string filenameWithoutExtension) {
    auto code = readFile(filenameWithoutExtension + ".bas");
    runCode(code);
}

TEST(CompilerTest, CrLfLineEndings) {
    // Don't assume any particular line ending in crlf.bas, although it should be LFs.
    auto code = readFile("crlf.bas");
    auto codeLf = boost::replace_all_copy(code, "\r\n", "\n");
    auto codeCrLf = boost::replace_all_copy(codeLf, "\n", "\r\n");

    // Just need to see that it compiles successfully.
    compiler::SourceProgram sourceProgram{};
    compiler::CompiledProgram program{};
    sourceProgram.loadFromContent(codeCrLf);
    compiler::compileProgram(sourceProgram, &program);
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define COMPILER_TEST(x) \
    TEST(CompilerTest, x) { run(#x); }

// Regenerate with: build/scripts/updateCompilerTest.sh
//--- auto-generated code below ---
COMPILER_TEST(Characters_1)
COMPILER_TEST(Chr)
COMPILER_TEST(CodePoints)
COMPILER_TEST(CodeUnit)
COMPILER_TEST(CodeUnits)
COMPILER_TEST(Concat)
COMPILER_TEST(ControlBounds_1)
COMPILER_TEST(ControlBounds_2)
COMPILER_TEST(ControlBounds_3)
COMPILER_TEST(DateFromParts)
COMPILER_TEST(DateTimeFromParts)
COMPILER_TEST(DateTimeOffsetFromParts)
COMPILER_TEST(Days)
COMPILER_TEST(ErrorCode_ErrorMessage)
COMPILER_TEST(FileExists)
COMPILER_TEST(First_doc)
COMPILER_TEST(First_number_1)
COMPILER_TEST(First_number_2)
COMPILER_TEST(First_string_1)
COMPILER_TEST(First_string_2)
COMPILER_TEST(HasValue_number_1)
COMPILER_TEST(HasValue_number_2)
COMPILER_TEST(HasValue_string_1)
COMPILER_TEST(HasValue_string_2)
COMPILER_TEST(Hours)
COMPILER_TEST(Last_number_1)
COMPILER_TEST(Last_number_2)
COMPILER_TEST(Last_string_1)
COMPILER_TEST(Last_string_2)
COMPILER_TEST(Len_list)
COMPILER_TEST(Len_string)
COMPILER_TEST(ListFill_doc)
COMPILER_TEST(ListFill_number_1)
COMPILER_TEST(ListFill_number_2)
COMPILER_TEST(ListFill_number_3)
COMPILER_TEST(ListFill_string_1)
COMPILER_TEST(ListFill_string_2)
COMPILER_TEST(ListFill_string_3)
COMPILER_TEST(Mid_doc)
COMPILER_TEST(Mid_number_1)
COMPILER_TEST(Mid_number_2)
COMPILER_TEST(Mid_number_3)
COMPILER_TEST(Mid_number_4)
COMPILER_TEST(Mid_number_5)
COMPILER_TEST(Mid_string_1)
COMPILER_TEST(Mid_string_2)
COMPILER_TEST(Mid_string_3)
COMPILER_TEST(Mid_string_4)
COMPILER_TEST(Mid_string_5)
COMPILER_TEST(Milliseconds)
COMPILER_TEST(Minutes)
COMPILER_TEST(PathCombine)
COMPILER_TEST(PathDirectoryName_PathFileName)
COMPILER_TEST(ReadFileBytes_WriteFileBytes)
COMPILER_TEST(ReadFileLines_WriteFileLines)
COMPILER_TEST(ReadFileText_WriteFileText_DeleteFile)
COMPILER_TEST(Rectangle_1)
COMPILER_TEST(Rectangle_2)
COMPILER_TEST(Seconds)
COMPILER_TEST(Skip_doc)
COMPILER_TEST(Skip_number_1)
COMPILER_TEST(Skip_number_2)
COMPILER_TEST(Skip_number_3)
COMPILER_TEST(Skip_string_1)
COMPILER_TEST(Skip_string_2)
COMPILER_TEST(Skip_string_3)
COMPILER_TEST(StringFromCodeUnits)
COMPILER_TEST(Take_doc)
COMPILER_TEST(TimeZoneFromName)
COMPILER_TEST(TotalDays_and_friends)
COMPILER_TEST(Value_number_1)
COMPILER_TEST(Value_number_2)
COMPILER_TEST(Value_string_1)
COMPILER_TEST(Value_string_2)
COMPILER_TEST(and_or_short_circuit)
COMPILER_TEST(assign_dotted_1)
COMPILER_TEST(assign_dotted_2)
COMPILER_TEST(assign_dotted_3)
COMPILER_TEST(assign_dotted_4)
COMPILER_TEST(assign_dotted_5)
COMPILER_TEST(assign_list)
COMPILER_TEST(assign_string)
COMPILER_TEST(binary_operators_boolean)
COMPILER_TEST(boolean_literal)
COMPILER_TEST(built_in_constants)
COMPILER_TEST(call_function_one_arg)
COMPILER_TEST(call_function_two_arg)
COMPILER_TEST(call_function_zero_arg)
COMPILER_TEST(call_sub)
COMPILER_TEST(call_sub_arg)
COMPILER_TEST(comment_after_global)
COMPILER_TEST(comment_after_sub)
COMPILER_TEST(comment_before_global)
COMPILER_TEST(comment_before_sub)
COMPILER_TEST(const_global)
COMPILER_TEST(const_global_cannot_assign)
COMPILER_TEST(const_local)
COMPILER_TEST(const_local_cannot_assign)
COMPILER_TEST(continue_mismatch)
COMPILER_TEST(continue_outside_loop)
COMPILER_TEST(convert_date_as_datetime)
COMPILER_TEST(convert_datetime_as_date)
COMPILER_TEST(convert_datetimeoffset_as_date)
COMPILER_TEST(convert_datetimeoffset_as_datetime)
COMPILER_TEST(convert_number_as_optional)
COMPILER_TEST(convert_number_as_string)
COMPILER_TEST(crlf)
COMPILER_TEST(dim_list_mismatched_yield_types)
COMPILER_TEST(dim_list_no_yields)
COMPILER_TEST(dim_list_number)
COMPILER_TEST(dim_list_string)
COMPILER_TEST(directories)
COMPILER_TEST(do)
COMPILER_TEST(do_continue)
COMPILER_TEST(do_exit)
COMPILER_TEST(do_invalid_condition_type)
COMPILER_TEST(empty_main)
COMPILER_TEST(exit_mismatch)
COMPILER_TEST(exit_outside_loop)
COMPILER_TEST(for_continue)
COMPILER_TEST(for_each_early_return)
COMPILER_TEST(for_each_object_list)
COMPILER_TEST(for_each_string)
COMPILER_TEST(for_each_value_list)
COMPILER_TEST(for_exit)
COMPILER_TEST(for_loop)
COMPILER_TEST(global_value_number)
COMPILER_TEST(global_value_string)
COMPILER_TEST(hello_world)
COMPILER_TEST(if_false)
COMPILER_TEST(input_string)
COMPILER_TEST(list_add_element)
COMPILER_TEST(list_add_list)
COMPILER_TEST(list_add_mismatched_list)
COMPILER_TEST(list_index)
COMPILER_TEST(list_set_1)
COMPILER_TEST(list_set_2)
COMPILER_TEST(main_not_first_in_program)
COMPILER_TEST(map_number_number_not_found)
COMPILER_TEST(map_number_number_set)
COMPILER_TEST(map_number_record_not_found)
COMPILER_TEST(map_number_record_set_1)
COMPILER_TEST(map_number_record_set_2)
COMPILER_TEST(map_number_record_set_3)
COMPILER_TEST(map_number_string_not_found)
COMPILER_TEST(map_number_string_set)
COMPILER_TEST(map_record_record_not_found)
COMPILER_TEST(map_record_record_set_1)
COMPILER_TEST(map_record_record_set_2)
COMPILER_TEST(map_record_record_set_3)
COMPILER_TEST(map_string_number_not_found)
COMPILER_TEST(map_string_number_set)
COMPILER_TEST(map_string_record_not_found)
COMPILER_TEST(map_string_record_set_1)
COMPILER_TEST(map_string_record_set_2)
COMPILER_TEST(map_string_record_set_3)
COMPILER_TEST(map_string_string_not_found)
COMPILER_TEST(map_string_string_set)
COMPILER_TEST(math_functions)
COMPILER_TEST(named_record_type_1)
COMPILER_TEST(named_record_type_2)
COMPILER_TEST(not_Boolean)
COMPILER_TEST(not_Number)
COMPILER_TEST(number_comparison)
COMPILER_TEST(optional_assign)
COMPILER_TEST(optional_parameter)
COMPILER_TEST(pascal_triangle)
COMPILER_TEST(plus_operator_list_of_list_of_number)
COMPILER_TEST(plus_operator_list_of_list_of_string)
COMPILER_TEST(plus_operator_list_of_string)
COMPILER_TEST(print)
COMPILER_TEST(print_number)
COMPILER_TEST(record_default)
COMPILER_TEST(record_field_get_1)
COMPILER_TEST(record_field_get_2)
COMPILER_TEST(record_field_get_3)
COMPILER_TEST(record_field_get_4)
COMPILER_TEST(record_field_get_5)
COMPILER_TEST(record_field_get_6)
COMPILER_TEST(record_field_get_7)
COMPILER_TEST(record_field_get_8)
COMPILER_TEST(record_field_set)
COMPILER_TEST(record_list_field)
COMPILER_TEST(record_literal)
COMPILER_TEST(record_named_1)
COMPILER_TEST(record_named_2)
COMPILER_TEST(record_named_3)
COMPILER_TEST(record_named_4)
COMPILER_TEST(record_named_5)
COMPILER_TEST(record_named_6)
COMPILER_TEST(record_named_7)
COMPILER_TEST(record_nested)
COMPILER_TEST(record_print)
COMPILER_TEST(rethrow)
COMPILER_TEST(select_case_list_of_number)
COMPILER_TEST(select_case_multiple_case_else)
COMPILER_TEST(select_case_number)
COMPILER_TEST(select_case_number_range)
COMPILER_TEST(select_case_string)
COMPILER_TEST(select_case_string_range)
COMPILER_TEST(select_case_type_mismatch)
COMPILER_TEST(string_concat)
COMPILER_TEST(throw_caught_in_same_sub)
COMPILER_TEST(throw_from_catch_block)
COMPILER_TEST(throw_from_function)
COMPILER_TEST(throw_from_sub)
COMPILER_TEST(throw_nested)
COMPILER_TEST(throw_uncaught)
COMPILER_TEST(throw_uncaught_without_code)
COMPILER_TEST(variable_string)
COMPILER_TEST(while_continue)
COMPILER_TEST(while_exit)
COMPILER_TEST(while_loop)
