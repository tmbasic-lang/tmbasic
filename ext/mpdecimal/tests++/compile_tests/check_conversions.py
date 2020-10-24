#!/usr/bin/env python

#
# Copyright (c) 2020 Stefan Krah. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#


import sys
import os
import subprocess
from tempfile import NamedTemporaryFile
from sys import platform


PREFIX = os.getcwd()


if platform == "win32":
    COMPILE = ["cl",
               "/I%s\\..\\..\\libmpdec" % PREFIX,
               "-I%s\\..\\..\\libmpdec++" % PREFIX,
               "-c"]
    DELETE = False
else:
    COMPILE = ["g++", "-Wall",  "-Wextra",  "-std=c++11", "-O0",
               "-I%s/../../libmpdec" % PREFIX,
               "-I%s/../../libmpdec++" % PREFIX,
               "-c"]
    DELETE = True


INCLUDES = """
#include <cstdint>
#include <climits>
#include <cstddef>
#include "mpdecimal.h"
#include "decimal.hh"

using decimal::Decimal;
"""


def check_nullary(templates, success=True):
    for t in templates:
        with NamedTemporaryFile(suffix=".cc", delete=DELETE) as f:
            f.write(bytes(t, "ascii"))
            f.flush()
            if DELETE is False:
                f.close()
            cmd = COMPILE + [f.name]
            p = subprocess.run(cmd, capture_output=True)
            if (p.returncode == 0) is not success:
                sys.stderr.write("%s\n\n" % t)
                sys.exit(1)
        if DELETE is False:
            os.remove(f.name)


def check_unary(templates, args, success=True):
    for t in templates:
        for arg in args:
            s = t % arg
            with NamedTemporaryFile(suffix=".cc", delete=DELETE) as f:
                f.write(bytes(s, "ascii"))
                f.flush()
                if DELETE is False:
                    f.close()
                cmd = COMPILE + [f.name]
                p = subprocess.run(cmd, capture_output=True)
                if (p.returncode == 0) is not success:
                    sys.stderr.write("%s\n\n" % s)
                    sys.exit(1)
            if DELETE is False:
                os.remove(f.name)


def check_binary(templates, operators, args, success=True):
    for t in templates:
        for op in operators:
            for arg in args:
                s = t.format(op=op, arg=arg)
                with NamedTemporaryFile(suffix=".cc", delete=DELETE) as f:
                    f.write(bytes(s, "ascii"))
                    f.flush()
                    if DELETE is False:
                        f.close()
                    cmd = COMPILE + [f.name]
                    p = subprocess.run(cmd, capture_output=True)
                    if (p.returncode == 0) is not success:
                        sys.stderr.write("%s\n\n" % s)
                        sys.exit(1)
                if DELETE is False:
                    os.remove(f.name)


# ==============================================================================
#                                   boolean
# ==============================================================================

# Test 'if' and !.
boolean_nullary_templates = [
  INCLUDES + "int main() { Decimal x = 0; if (x) {;} }",
  INCLUDES + "int main() { Decimal x = 1; if (x) {;} }",
  INCLUDES + "int main() { bool b = !Decimal(0); }",
  INCLUDES + "int main() { bool b = !Decimal(1); }",
]

# Test && and ||.
boolean_binary_templates = [
  INCLUDES + "int main() {{ bool b = {arg} {op} Decimal(10); }}",
  INCLUDES + "int main() {{ bool b = Decimal(10) {op} {arg}; }}",
]

disabled_boolean_binary_operators = ["&&", "||"]


def check_disabled_boolean_nullary():
    check_nullary(boolean_nullary_templates,
                  success=False)


def check_disabled_boolean_binary():
    check_binary(boolean_binary_templates,
                 disabled_boolean_binary_operators,
                 enabled_conversion_args,
                 success=False)


# ==============================================================================
#                                  conversions
# ==============================================================================

enabled_conversion_templates = [
  INCLUDES + "int main() { Decimal x = %s; }",
  INCLUDES + "int main() { Decimal x(%s); }",
  INCLUDES + "int main() { Decimal x{%s}; }",
  INCLUDES + "void f(Decimal x) { (void)x; } int main() { f(%s); }",
  INCLUDES + "Decimal f(void) { return %s; } int main() { (void)f(); }",
]

disabled_conversion_templates = [
  INCLUDES + "int main() { Decimal x = %s; }",
  INCLUDES + "void f(Decimal x) { (void)x; } int main() { f(%s); }",
  INCLUDES + "Decimal f(void) { return %s; } int main() { (void)f(); }",
  INCLUDES + "int main() { Decimal *x = new Decimal(); Decimal y = Decimal(x); delete x; /* %s */ }",
  INCLUDES + "int main() { Decimal *x = new Decimal(); Decimal y = x; delete x; /* %s */ }",
]

enabled_conversion_args = [
  "((unsigned char)10)",
  "((unsigned short)10)",
  "((unsigned int)10)",
  "((unsigned long)10)",
  "((unsigned long long)10)",

  "((uint8_t)UINT8_MAX)",
  "((uint16_t)UINT16_MAX)",
  "((uint32_t)UINT32_MAX)",
  "((uint64_t)UINT64_MAX)",

  "((uintptr_t)10)",

  "((signed char)-10)",
  "((short)-10)",
  "((int)-10)",
  "((long)-10)",
  "((long long)-10)",

  "((signed char)10)",
  "((short)10)",
  "((int)10)",
  "((long)10)",
  "((long long)10)",

  "((int8_t)INT8_MIN)",
  "((int16_t)INT16_MIN)",
  "((int32_t)INT32_MIN)",
  "((int64_t)INT64_MIN)",

  "((int8_t)INT8_MAX)",
  "((int16_t)INT16_MAX)",
  "((int32_t)INT32_MAX)",
  "((int64_t)INT64_MAX)",

  "((intptr_t)-10)",
  "((intptr_t)10)",

  "Decimal(10)",
]

disabled_conversion_args = [
  "nullptr",
  "true",
  "false",

  "((wchar_t)10)",
  "((char8_t)10)",
  "((char16_t)10)",
  "((char32_t)10)",

  "((float)10)",
  "((double)10)",
  "((long double)10)",

  "\"10\"",
  "std::string(\"10\")"
]


def check_enabled_conversions():
    return check_unary(enabled_conversion_templates,
                       enabled_conversion_args,
                       success=True)


def check_disabled_conversions():
    return check_unary(disabled_conversion_templates,
                       disabled_conversion_args,
                       success=False)


# ==============================================================================
#                                unary operators
# ==============================================================================

left_unary_operator_templates = [
  INCLUDES + "int main() { Decimal x = %sDecimal(1); }",
]

right_unary_operator_templates = [
  INCLUDES + "int main() { Decimal x = Decimal(1)%s; }",
]

enabled_unary_operators = ["+", "-"]
disabled_unary_operators = ["++", "--", "!", "~", "*", "&"]


left_pointer_unary_operator_templates = [
  INCLUDES + "int main() { Decimal *x = new Decimal; %sx; delete x; }",
]

disabled_pointer_unary_operators = ["-", "~"]


def check_enabled_unary_op():
    check_unary(left_unary_operator_templates,
                enabled_unary_operators,
                success=True)


def check_disabled_unary_op():
    check_unary(left_unary_operator_templates,
                disabled_unary_operators,
                success=False)

    check_unary(right_unary_operator_templates,
                enabled_unary_operators,
                success=False)

    check_unary(right_unary_operator_templates,
                disabled_unary_operators,
                success=False)


def check_disabled_pointer_unary_op():
    check_unary(left_pointer_unary_operator_templates,
                disabled_pointer_unary_operators,
                success=False)


# ==============================================================================
#                              assignment operators
# ==============================================================================

assignment_operator_templates = [
  INCLUDES + "int main() {{ Decimal x = 10; x {op} {arg}; }}",
]

pointer_assignment_operator_templates = [
  INCLUDES + "int main() {{ Decimal *d = new Decimal; Decimal *x = d; x {op} {arg}; delete d; }}",
]

enabled_assignment_operators = ["=", "+=", "-=", "*=", "/=", "%="]
disabled_assignment_operators = ["&=", "|=", "^=", "<<=", ">>="]

disabled_pointer_assignment_operators = ["*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="]

def check_enabled_assignment_op():
    check_binary(assignment_operator_templates,
                 enabled_assignment_operators,
                 enabled_conversion_args,
                 success=True)


def check_disabled_assignment_op():
    check_binary(assignment_operator_templates,
                 enabled_assignment_operators,
                 disabled_conversion_args,
                 success=False)

    check_binary(assignment_operator_templates,
                 disabled_assignment_operators,
                 enabled_conversion_args,
                 success=False)


def check_disabled_pointer_assignment_op():
    check_binary(pointer_assignment_operator_templates,
                 disabled_pointer_assignment_operators,
                 enabled_conversion_args,
                 success=False)


# ==============================================================================
#                              comparison operators
# ==============================================================================

comparison_operator_templates = [
  INCLUDES + "int main() {{ bool b = {arg} {op} Decimal(10); }}",
  INCLUDES + "int main() {{ bool b = Decimal(10) {op} {arg}; }}",
]

enabled_comparison_operators = ["==", "!=", "<", "<=", ">=", ">"]
disabled_comparison_operators = ["<==>"]


def check_enabled_comparison_op():
    check_binary(comparison_operator_templates,
                 enabled_comparison_operators,
                 enabled_conversion_args,
                 success=True)


def check_disabled_comparison_op():
    check_binary(comparison_operator_templates,
                 enabled_comparison_operators,
                 disabled_conversion_args,
                 success=False)

    check_binary(comparison_operator_templates,
                 disabled_comparison_operators,
                 enabled_conversion_args,
                 success=False)


# ==============================================================================
#                              arithmetic operators
# ==============================================================================

arithmetic_operator_templates = [
  INCLUDES + "int main() {{ Decimal b = {arg} {op} Decimal(10); }}",
  INCLUDES + "int main() {{ Decimal b = Decimal(10) {op} {arg}; }}",
]

pointer_arithmetic_operator_templates = [
  INCLUDES + "int main() {{ Decimal b = 10; Decimal *x = &b; {arg} {op} x; }} /* %s */",
  INCLUDES + "int main() {{ Decimal b = 10; Decimal *x = &b; x {op} {arg}; }} /* %s */",
]

enabled_arithmetic_operators = ["+", "-", "*", "/", "%"]
disabled_arithmetic_operators = ["&", "|", "^", "<<", ">>"]

disabled_pointer_arithmetic_operators = ["*", "/", "%", "&", "|", "^", "<<", ">>"]


def check_enabled_arithmetic_op():
    check_binary(arithmetic_operator_templates,
                 enabled_arithmetic_operators,
                 enabled_conversion_args,
                 success=True)


def check_disabled_arithmetic_op():
    check_binary(arithmetic_operator_templates,
                 enabled_arithmetic_operators,
                 disabled_conversion_args,
                 success=False)

    check_binary(arithmetic_operator_templates,
                 disabled_arithmetic_operators,
                 enabled_conversion_args,
                 success=False)


def check_disabled_pointer_arithmetic_op():
    check_binary(pointer_arithmetic_operator_templates,
                 disabled_pointer_arithmetic_operators,
                 enabled_conversion_args,
                 success=False)


if __name__ == "__main__":
    check_disabled_boolean_nullary()
    check_disabled_boolean_binary()

    check_enabled_conversions()
    check_disabled_conversions()

    check_enabled_unary_op()
    check_disabled_unary_op()
    check_disabled_pointer_unary_op()

    check_enabled_assignment_op()
    check_disabled_assignment_op()
    check_disabled_pointer_assignment_op()

    check_enabled_comparison_op()
    check_disabled_comparison_op()

    check_enabled_arithmetic_op()
    check_disabled_arithmetic_op()

    check_disabled_pointer_arithmetic_op()

    sys.exit(0)
