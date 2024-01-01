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
    std::optional<std::string> errorMessage{};
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
        errorMessage = ex.message;
    }

    if (compileSuccess) {
        try {
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
        } catch (std::exception& ex) {
            consoleOutputStream << "Uncaught runtime error\n" << ex.what() << "\n";
        }
    }

    auto actualOutput = consoleOutputStream.str();

    // If we're about to fail, then also print the error message.
    // It's likely that this message is more useful than the assert failure message.
    if (actualOutput != expectedOutput && errorMessage.has_value()) {
        std::cerr << "Error message:\n" << *errorMessage << "\n\n";
    }

    ASSERT_EQ(expectedOutput, actualOutput);

    Interpreter::printDebugTimings();
}

static void run(string dir, string filenameWithoutExtension) {
    auto code = readFile(dir + "/" + filenameWithoutExtension + ".bas");
    runCode(code);
}

TEST(CompilerTest, CrLfLineEndings) {
    // Don't assume any particular line ending in crlf.bas, although it should be LFs.
    auto code = readFile("strings/crlf.bas");
    auto codeLf = boost::replace_all_copy(code, "\r\n", "\n");
    auto codeCrLf = boost::replace_all_copy(codeLf, "\n", "\r\n");

    // Just need to see that it compiles successfully.
    compiler::SourceProgram sourceProgram{};
    compiler::CompiledProgram program{};
    sourceProgram.loadFromContent(codeCrLf);
    compiler::compileProgram(sourceProgram, &program);
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define COMPILER_TEST(dir, file) \
    TEST(CompilerTest, dir##__##file) { run(#dir, #file); }

// Regenerate with: build/scripts/updateCompilerTest.sh
//--- auto-generated code below ---
COMPILER_TEST(assignments, assign_dotted_1)
COMPILER_TEST(assignments, assign_dotted_2)
COMPILER_TEST(assignments, assign_dotted_3)
COMPILER_TEST(assignments, assign_dotted_4)
COMPILER_TEST(assignments, assign_dotted_5)
COMPILER_TEST(assignments, assign_list)
COMPILER_TEST(assignments, assign_string)
COMPILER_TEST(assignments, variable_shadowing_function)
COMPILER_TEST(assignments, variable_string)
COMPILER_TEST(booleans, and_or_short_circuit)
COMPILER_TEST(booleans, binary_operators_boolean)
COMPILER_TEST(booleans, boolean_literal)
COMPILER_TEST(booleans, if_false)
COMPILER_TEST(booleans, not_Boolean)
COMPILER_TEST(console, print_anonymous_record)
COMPILER_TEST(console, print_boolean)
COMPILER_TEST(console, print_date)
COMPILER_TEST(console, print_datetime)
COMPILER_TEST(console, print_datetimeoffset)
COMPILER_TEST(console, print_list_number)
COMPILER_TEST(console, print_map_number_to_number)
COMPILER_TEST(console, print_named_record)
COMPILER_TEST(console, print_number)
COMPILER_TEST(console, print_optional_number)
COMPILER_TEST(console, print_set_number)
COMPILER_TEST(console, print_string)
COMPILER_TEST(constants, built_in_constants)
COMPILER_TEST(constants, const_global)
COMPILER_TEST(constants, const_global_cannot_assign)
COMPILER_TEST(constants, const_local)
COMPILER_TEST(constants, const_local_cannot_assign)
COMPILER_TEST(dates, DateFromParts)
COMPILER_TEST(dates, DateTimeFromParts)
COMPILER_TEST(dates, DateTimeOffsetFromParts)
COMPILER_TEST(dates, Day_Date)
COMPILER_TEST(dates, Day_DateTime)
COMPILER_TEST(dates, Day_DateTimeOffset)
COMPILER_TEST(dates, Days)
COMPILER_TEST(dates, Hour_DateTime)
COMPILER_TEST(dates, Hour_DateTimeOffset)
COMPILER_TEST(dates, Hours)
COMPILER_TEST(dates, Millisecond_DateTime)
COMPILER_TEST(dates, Millisecond_DateTimeOffset)
COMPILER_TEST(dates, Milliseconds)
COMPILER_TEST(dates, Minute_DateTime)
COMPILER_TEST(dates, Minute_DateTimeOffset)
COMPILER_TEST(dates, Minutes)
COMPILER_TEST(dates, Month_Date)
COMPILER_TEST(dates, Month_DateTime)
COMPILER_TEST(dates, Month_DateTimeOffset)
COMPILER_TEST(dates, Second_DateTime)
COMPILER_TEST(dates, Seconds)
COMPILER_TEST(dates, TimeZoneFromName)
COMPILER_TEST(dates, TotalDays_and_friends)
COMPILER_TEST(dates, Year_Date)
COMPILER_TEST(dates, Year_DateTime)
COMPILER_TEST(dates, Year_DateTimeOffset)
COMPILER_TEST(dates, convert_date_as_datetime)
COMPILER_TEST(dates, convert_datetime_as_date)
COMPILER_TEST(dates, convert_datetimeoffset_as_date)
COMPILER_TEST(dates, convert_datetimeoffset_as_datetime)
COMPILER_TEST(disk, FileExists)
COMPILER_TEST(disk, PathCombine)
COMPILER_TEST(disk, PathDirectoryName_PathFileName)
COMPILER_TEST(disk, ReadFileBytes_WriteFileBytes)
COMPILER_TEST(disk, ReadFileLines_WriteFileLines)
COMPILER_TEST(disk, ReadFileText_WriteFileText_DeleteFile)
COMPILER_TEST(disk, directories)
COMPILER_TEST(errors, ErrorCode_ErrorMessage)
COMPILER_TEST(errors, rethrow)
COMPILER_TEST(errors, throw_caught_in_same_sub)
COMPILER_TEST(errors, throw_from_catch_block)
COMPILER_TEST(errors, throw_from_function)
COMPILER_TEST(errors, throw_from_sub)
COMPILER_TEST(errors, throw_nested)
COMPILER_TEST(errors, throw_uncaught)
COMPILER_TEST(errors, throw_uncaught_without_code)
COMPILER_TEST(forms, ControlBounds_1)
COMPILER_TEST(forms, ControlBounds_2)
COMPILER_TEST(forms, ControlBounds_3)
COMPILER_TEST(forms, Rectangle_1)
COMPILER_TEST(forms, Rectangle_2)
COMPILER_TEST(if, if_else_if)
COMPILER_TEST(if, if_else_if_else)
COMPILER_TEST(if, if_only)
COMPILER_TEST(if, if_short_form)
COMPILER_TEST(input, input_date)
COMPILER_TEST(input, input_number_1)
COMPILER_TEST(input, input_number_2)
COMPILER_TEST(input, input_string_1)
COMPILER_TEST(input, input_string_2)
COMPILER_TEST(lists, Concat)
COMPILER_TEST(lists, First_doc)
COMPILER_TEST(lists, First_number_1)
COMPILER_TEST(lists, First_number_2)
COMPILER_TEST(lists, First_string_1)
COMPILER_TEST(lists, First_string_2)
COMPILER_TEST(lists, Last_number_1)
COMPILER_TEST(lists, Last_number_2)
COMPILER_TEST(lists, Last_string_1)
COMPILER_TEST(lists, Last_string_2)
COMPILER_TEST(lists, Len_list)
COMPILER_TEST(lists, ListFill_doc)
COMPILER_TEST(lists, ListFill_number_1)
COMPILER_TEST(lists, ListFill_number_2)
COMPILER_TEST(lists, ListFill_number_3)
COMPILER_TEST(lists, ListFill_string_1)
COMPILER_TEST(lists, ListFill_string_2)
COMPILER_TEST(lists, ListFill_string_3)
COMPILER_TEST(lists, Mid_doc)
COMPILER_TEST(lists, Mid_number_1)
COMPILER_TEST(lists, Mid_number_2)
COMPILER_TEST(lists, Mid_number_3)
COMPILER_TEST(lists, Mid_number_4)
COMPILER_TEST(lists, Mid_number_5)
COMPILER_TEST(lists, Mid_string_1)
COMPILER_TEST(lists, Mid_string_2)
COMPILER_TEST(lists, Mid_string_3)
COMPILER_TEST(lists, Mid_string_4)
COMPILER_TEST(lists, Mid_string_5)
COMPILER_TEST(lists, RemoveAt_multiple_1)
COMPILER_TEST(lists, RemoveAt_multiple_2)
COMPILER_TEST(lists, RemoveAt_multiple_3)
COMPILER_TEST(lists, RemoveAt_multiple_4)
COMPILER_TEST(lists, RemoveAt_single_1)
COMPILER_TEST(lists, RemoveAt_single_2)
COMPILER_TEST(lists, RemoveAt_single_3)
COMPILER_TEST(lists, Skip_doc)
COMPILER_TEST(lists, Skip_number_1)
COMPILER_TEST(lists, Skip_number_2)
COMPILER_TEST(lists, Skip_number_3)
COMPILER_TEST(lists, Skip_string_1)
COMPILER_TEST(lists, Skip_string_2)
COMPILER_TEST(lists, Skip_string_3)
COMPILER_TEST(lists, Take_doc)
COMPILER_TEST(lists, dim_list_mismatched_yield_types)
COMPILER_TEST(lists, dim_list_no_yields)
COMPILER_TEST(lists, dim_list_number)
COMPILER_TEST(lists, dim_list_string)
COMPILER_TEST(lists, dim_list_with_yield_map)
COMPILER_TEST(lists, list_add_element)
COMPILER_TEST(lists, list_add_list)
COMPILER_TEST(lists, list_add_mismatched_list)
COMPILER_TEST(lists, list_index)
COMPILER_TEST(lists, list_of_list_of_number)
COMPILER_TEST(lists, list_set_1)
COMPILER_TEST(lists, list_set_2)
COMPILER_TEST(lists, plus_operator_list_of_list_of_number)
COMPILER_TEST(lists, plus_operator_list_of_list_of_string)
COMPILER_TEST(lists, plus_operator_list_of_string)
COMPILER_TEST(lists, yield_number_and_print)
COMPILER_TEST(lists, yield_outside_dim_list)
COMPILER_TEST(loops, continue_mismatch)
COMPILER_TEST(loops, continue_outside_loop)
COMPILER_TEST(loops, do)
COMPILER_TEST(loops, do_continue)
COMPILER_TEST(loops, do_exit)
COMPILER_TEST(loops, do_invalid_condition_type)
COMPILER_TEST(loops, exit_mismatch)
COMPILER_TEST(loops, exit_outside_loop)
COMPILER_TEST(loops, for_continue)
COMPILER_TEST(loops, for_each_early_return)
COMPILER_TEST(loops, for_each_object_list)
COMPILER_TEST(loops, for_each_string)
COMPILER_TEST(loops, for_each_value_list)
COMPILER_TEST(loops, for_exit)
COMPILER_TEST(loops, for_loop)
COMPILER_TEST(loops, while_continue)
COMPILER_TEST(loops, while_exit)
COMPILER_TEST(loops, while_loop)
COMPILER_TEST(maps, ContainsKey_doc_1)
COMPILER_TEST(maps, ContainsKey_map_number_to_number_1)
COMPILER_TEST(maps, ContainsKey_map_number_to_number_2)
COMPILER_TEST(maps, ContainsKey_map_number_to_number_3)
COMPILER_TEST(maps, ContainsKey_map_string_to_number_1)
COMPILER_TEST(maps, ContainsKey_map_string_to_number_2)
COMPILER_TEST(maps, ContainsKey_map_string_to_number_3)
COMPILER_TEST(maps, Find_map_number_to_number_1)
COMPILER_TEST(maps, Find_map_number_to_number_2)
COMPILER_TEST(maps, Find_map_number_to_string_1)
COMPILER_TEST(maps, Find_map_number_to_string_2)
COMPILER_TEST(maps, Find_map_string_to_number_1)
COMPILER_TEST(maps, Find_map_string_to_number_2)
COMPILER_TEST(maps, Find_map_string_to_string_1)
COMPILER_TEST(maps, Find_map_string_to_string_2)
COMPILER_TEST(maps, dim_map_with_yield_list)
COMPILER_TEST(maps, map_number_number_not_found)
COMPILER_TEST(maps, map_number_number_set)
COMPILER_TEST(maps, map_number_record_not_found)
COMPILER_TEST(maps, map_number_record_set_1)
COMPILER_TEST(maps, map_number_record_set_2)
COMPILER_TEST(maps, map_number_record_set_3)
COMPILER_TEST(maps, map_number_string_not_found)
COMPILER_TEST(maps, map_number_string_set)
COMPILER_TEST(maps, map_record_record_not_found)
COMPILER_TEST(maps, map_record_record_set_1)
COMPILER_TEST(maps, map_record_record_set_2)
COMPILER_TEST(maps, map_record_record_set_3)
COMPILER_TEST(maps, map_string_number_not_found)
COMPILER_TEST(maps, map_string_number_set)
COMPILER_TEST(maps, map_string_record_not_found)
COMPILER_TEST(maps, map_string_record_set_1)
COMPILER_TEST(maps, map_string_record_set_2)
COMPILER_TEST(maps, map_string_record_set_3)
COMPILER_TEST(maps, map_string_string_not_found)
COMPILER_TEST(maps, map_string_string_set)
COMPILER_TEST(misc, comment_after_global)
COMPILER_TEST(misc, comment_after_sub)
COMPILER_TEST(misc, comment_before_global)
COMPILER_TEST(misc, comment_before_sub)
COMPILER_TEST(misc, empty_main)
COMPILER_TEST(misc, global_value_number)
COMPILER_TEST(misc, global_value_string)
COMPILER_TEST(misc, hello_world)
COMPILER_TEST(misc, input_string)
COMPILER_TEST(misc, main_not_first_in_program)
COMPILER_TEST(misc, pascal_triangle)
COMPILER_TEST(numbers, Ceil)
COMPILER_TEST(numbers, Floor)
COMPILER_TEST(numbers, convert_number_as_string)
COMPILER_TEST(numbers, math_functions)
COMPILER_TEST(numbers, not_Number)
COMPILER_TEST(numbers, number_comparison)
COMPILER_TEST(optionals, HasValue_number_1)
COMPILER_TEST(optionals, HasValue_number_2)
COMPILER_TEST(optionals, HasValue_string_1)
COMPILER_TEST(optionals, HasValue_string_2)
COMPILER_TEST(optionals, Value_number_1)
COMPILER_TEST(optionals, Value_number_2)
COMPILER_TEST(optionals, Value_string_1)
COMPILER_TEST(optionals, Value_string_2)
COMPILER_TEST(optionals, convert_number_as_optional)
COMPILER_TEST(optionals, optional_assign)
COMPILER_TEST(optionals, optional_parameter)
COMPILER_TEST(print, print)
COMPILER_TEST(print, print_number)
COMPILER_TEST(procedure_calls, call_function_one_arg)
COMPILER_TEST(procedure_calls, call_function_two_arg)
COMPILER_TEST(procedure_calls, call_function_zero_arg)
COMPILER_TEST(procedure_calls, call_sub)
COMPILER_TEST(procedure_calls, call_sub_arg)
COMPILER_TEST(procedure_calls, early_return_dim_list_1)
COMPILER_TEST(procedure_calls, early_return_dim_list_2)
COMPILER_TEST(procedure_calls, early_return_dim_list_3)
COMPILER_TEST(procedure_calls, early_return_dim_map_1)
COMPILER_TEST(procedure_calls, early_return_dim_map_2)
COMPILER_TEST(procedure_calls, early_return_dim_map_3)
COMPILER_TEST(procedure_calls, early_return_dim_set_1)
COMPILER_TEST(procedure_calls, early_return_dim_set_2)
COMPILER_TEST(procedure_calls, early_return_dim_set_3)
COMPILER_TEST(procedure_calls, early_return_do_1)
COMPILER_TEST(procedure_calls, early_return_do_2)
COMPILER_TEST(procedure_calls, early_return_do_3)
COMPILER_TEST(procedure_calls, early_return_do_4)
COMPILER_TEST(procedure_calls, early_return_do_5)
COMPILER_TEST(procedure_calls, early_return_do_exit_1)
COMPILER_TEST(procedure_calls, early_return_do_exit_2)
COMPILER_TEST(procedure_calls, early_return_do_exit_3)
COMPILER_TEST(procedure_calls, early_return_do_exit_4)
COMPILER_TEST(procedure_calls, early_return_do_exit_5)
COMPILER_TEST(procedure_calls, early_return_if_1)
COMPILER_TEST(procedure_calls, early_return_if_2)
COMPILER_TEST(procedure_calls, early_return_if_3)
COMPILER_TEST(procedure_calls, early_return_if_4)
COMPILER_TEST(procedure_calls, early_return_select_case_1)
COMPILER_TEST(procedure_calls, early_return_select_case_2)
COMPILER_TEST(procedure_calls, early_return_select_case_3)
COMPILER_TEST(procedure_calls, early_return_select_case_4)
COMPILER_TEST(procedure_calls, early_return_select_case_5)
COMPILER_TEST(procedure_calls, early_return_select_case_6)
COMPILER_TEST(procedure_calls, early_return_select_case_7)
COMPILER_TEST(procedure_calls, early_return_select_case_8)
COMPILER_TEST(procedure_calls, early_return_try_1)
COMPILER_TEST(procedure_calls, early_return_try_2)
COMPILER_TEST(procedure_calls, early_return_try_3)
COMPILER_TEST(procedure_calls, early_return_try_4)
COMPILER_TEST(procedure_calls, early_return_try_5)
COMPILER_TEST(procedure_calls, early_return_try_6)
COMPILER_TEST(procedure_calls, early_return_try_7)
COMPILER_TEST(procedure_calls, early_return_try_8)
COMPILER_TEST(procedure_calls, early_return_while_false_1)
COMPILER_TEST(procedure_calls, early_return_while_false_2)
COMPILER_TEST(procedure_calls, early_return_while_true_1)
COMPILER_TEST(procedure_calls, early_return_while_true_2)
COMPILER_TEST(procedure_calls, function_with_no_parameters_1)
COMPILER_TEST(procedure_calls, function_with_no_parameters_2)
COMPILER_TEST(procedure_calls, missing_return_empty_function)
COMPILER_TEST(procedure_calls, return_nothing_in_function)
COMPILER_TEST(procedure_calls, return_value_in_subroutine)
COMPILER_TEST(procedure_calls, wrong_return_type)
COMPILER_TEST(records, anonymous_to_named_record_conversion_inside_for)
COMPILER_TEST(records, anonymous_to_named_record_conversion_inside_foreach)
COMPILER_TEST(records, anonymous_to_named_record_conversion_inside_if)
COMPILER_TEST(records, anonymous_to_named_record_conversion_inside_while)
COMPILER_TEST(records, anonymous_to_named_record_conversion_outside_block)
COMPILER_TEST(records, list_of_named_record_explicit_conversion)
COMPILER_TEST(records, named_record_type_1)
COMPILER_TEST(records, named_record_type_2)
COMPILER_TEST(records, record_default)
COMPILER_TEST(records, record_field_get_1)
COMPILER_TEST(records, record_field_get_2)
COMPILER_TEST(records, record_field_get_3)
COMPILER_TEST(records, record_field_get_4)
COMPILER_TEST(records, record_field_get_5)
COMPILER_TEST(records, record_field_get_6)
COMPILER_TEST(records, record_field_get_7)
COMPILER_TEST(records, record_field_get_8)
COMPILER_TEST(records, record_field_set)
COMPILER_TEST(records, record_list_field)
COMPILER_TEST(records, record_literal)
COMPILER_TEST(records, record_named_1)
COMPILER_TEST(records, record_named_2)
COMPILER_TEST(records, record_named_3)
COMPILER_TEST(records, record_named_4)
COMPILER_TEST(records, record_named_5)
COMPILER_TEST(records, record_named_6)
COMPILER_TEST(records, record_named_7)
COMPILER_TEST(records, record_nested)
COMPILER_TEST(records, record_print)
COMPILER_TEST(records, return_named_record_1)
COMPILER_TEST(records, return_named_record_2)
COMPILER_TEST(records, return_named_record_explicitly_converted_to_optional)
COMPILER_TEST(records, return_named_record_implicitly_converted_to_optional)
COMPILER_TEST(samples, advent_of_code_2023_day1)
COMPILER_TEST(samples, advent_of_code_2023_day2)
COMPILER_TEST(samples, advent_of_code_2023_day3_part1)
COMPILER_TEST(samples, advent_of_code_2023_day3_part2)
COMPILER_TEST(samples, advent_of_code_2023_day4_part1)
COMPILER_TEST(samples, advent_of_code_2023_day4_part2)
COMPILER_TEST(samples, advent_of_code_2023_day5_part1)
COMPILER_TEST(samples, advent_of_code_2023_day5_part2)
COMPILER_TEST(samples, advent_of_code_2023_day6_part1)
COMPILER_TEST(select_case, select_case_list_of_number)
COMPILER_TEST(select_case, select_case_multiple_case_else)
COMPILER_TEST(select_case, select_case_number)
COMPILER_TEST(select_case, select_case_number_range)
COMPILER_TEST(select_case, select_case_string)
COMPILER_TEST(select_case, select_case_string_range)
COMPILER_TEST(select_case, select_case_type_mismatch)
COMPILER_TEST(sets, Contains_set_number)
COMPILER_TEST(sets, Contains_set_string)
COMPILER_TEST(sets, Set_number_empty)
COMPILER_TEST(sets, Set_number_minus_operator)
COMPILER_TEST(sets, Set_number_plus_operator_1)
COMPILER_TEST(sets, Set_number_plus_operator_2)
COMPILER_TEST(sets, Set_string_empty)
COMPILER_TEST(sets, Set_string_minus_operator)
COMPILER_TEST(sets, Set_string_plus_operator_1)
COMPILER_TEST(sets, Set_string_plus_operator_2)
COMPILER_TEST(sets, Values_set_1)
COMPILER_TEST(sets, Values_set_2)
COMPILER_TEST(sets, dim_set_duplicate_number)
COMPILER_TEST(sets, dim_set_duplicate_string)
COMPILER_TEST(strings, Characters_1)
COMPILER_TEST(strings, Chr)
COMPILER_TEST(strings, CodePoints)
COMPILER_TEST(strings, CodeUnit)
COMPILER_TEST(strings, CodeUnits)
COMPILER_TEST(strings, IsDigit)
COMPILER_TEST(strings, Len_string)
COMPILER_TEST(strings, ParseNumber_1)
COMPILER_TEST(strings, ParseNumber_2)
COMPILER_TEST(strings, ParseNumber_3)
COMPILER_TEST(strings, Replace_1)
COMPILER_TEST(strings, Replace_2)
COMPILER_TEST(strings, Split_1)
COMPILER_TEST(strings, Split_2)
COMPILER_TEST(strings, StringFromCodeUnits)
COMPILER_TEST(strings, crlf)
COMPILER_TEST(strings, string_concat)
COMPILER_TEST(variables, dim_list_scopes)
COMPILER_TEST(variables, dim_map_scopes)
COMPILER_TEST(variables, dim_set_scopes)
COMPILER_TEST(variables, if_scopes)
COMPILER_TEST(variables, nested_scopes)
COMPILER_TEST(variables, try_scopes)
