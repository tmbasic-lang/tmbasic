/*
 * Copyright (c) 2020 Stefan Krah. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#include <cctype>
#include <cerrno>
#include <cinttypes>
#include <clocale>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "malloc_fail.hh"
#include "mpdecimal.h"

#include "common.hh"
#include "decimal.hh"


using decimal::ValueError;
using decimal::MallocError;
using decimal::RuntimeError;
using decimal::InvalidOperation;
using decimal::Decimal;
using decimal::Context;
using decimal::context_template;
using decimal::context;


namespace test {

enum skip_cmp {SKIP_NONE, SKIP_NAN, SKIP_NONINT};

const mpd_context_t readcontext {
#if defined(CONFIG_64)
  1070000000000000000,   /* prec */
  1070000000000000000,   /* emax */
  -1070000000000000000,  /* emin */
#elif defined(CONFIG_32)
  /* These ranges are needed for the official testsuite
   * and are generally not problematic at all. */
  1070000000,            /* prec */
  1070000000,            /* emax */
  -1070000000,           /* emin */
#else
  #error "config not defined"
#endif
  MPD_Malloc_error,      /* traps */
  0,                     /* status */
  0,                     /* newtrap */
  MPD_ROUND_HALF_UP,     /* round */
  0,                     /* clamp */
  1,                     /* allcr */
};

const mpd_context_t testcontext {
#if defined(CONFIG_64)
  999999999,               /* prec */
  MPD_MAX_EMAX,            /* emax */
  MPD_MIN_EMIN,            /* emin */
#elif defined(CONFIG_32)
  /* These ranges are needed for the official testsuite
   * and are generally not problematic at all. */
  999999999,               /* prec */
  999999999,               /* emax */
  -999999999,              /* emin */
#else
  #error "config not defined"
#endif
    MPD_Malloc_error,      /* traps */
    0,                     /* status */
    0,                     /* newtrap */
    MPD_ROUND_HALF_UP,     /* round */
    0,                     /* clamp */
    1,                     /* allcr */
};

/* Known differences that are within the spec */
struct result_diff {
    const char *id;
    const char *calc;
    const char *expected;
};

struct status_diff {
    const char *id;
    uint32_t calc;
    uint32_t expected;
};

const struct result_diff ulp_cases[] {
    /* Cases where the result is allowed to differ by less than one ULP.
     * Only needed if ctx->allcr is 0. */
    { "expx013", "1.001000", "1.001001" },
    { "expx020", "1.000000", "1.000001" },
    { "expx109", "0.999999910000004049999878", "0.999999910000004049999879" },
    { "expx1036", "1.005088", "1.005087" },
    { "expx350", "1.0000000", "1.0000001" },
    { "expx351", "1.0000000", "1.0000001" },
    { "expx352", "1.0000000", "1.0000001" },
};

const struct status_diff status_cases[] {
    /* With a reduced working precision in mpd_qpow() the status matches. */
    { "pwsx803", MPD_Inexact|MPD_Rounded|MPD_Subnormal|MPD_Underflow, MPD_Inexact|MPD_Rounded },
};

const char *skipit[] {
    /* NULL reference, decimal16, decimal32, or decimal128 */
    "absx900",
    "addx9990",
    "addx9991",
    "clam090",
    "clam091",
    "clam092",
    "clam093",
    "clam094",
    "clam095",
    "clam096",
    "clam097",
    "clam098",
    "clam099",
    "clam189",
    "clam190",
    "clam191",
    "clam192",
    "clam193",
    "clam194",
    "clam195",
    "clam196",
    "clam197",
    "clam198",
    "clam199",
    "comx990",
    "comx991",
    "cotx9990",
    "cotx9991",
    "ctmx9990",
    "ctmx9991",
    "ddabs900",
    "ddadd9990",
    "ddadd9991",
    "ddcom9990",
    "ddcom9991",
    "ddcot9990",
    "ddcot9991",
    "ddctm9990",
    "ddctm9991",
    "dddiv9998",
    "dddiv9999",
    "dddvi900",
    "dddvi901",
    "ddfma2990",
    "ddfma2991",
    "ddfma39990",
    "ddfma39991",
    "ddlogb900",
    "ddmax900",
    "ddmax901",
    "ddmxg900",
    "ddmxg901",
    "ddmin900",
    "ddmin901",
    "ddmng900",
    "ddmng901",
    "ddmul9990",
    "ddmul9991",
    "ddnextm900",
    "ddnextm900",
    "ddnextp900",
    "ddnextp900",
    "ddnextt900",
    "ddnextt901",
    "ddqua998",
    "ddqua999",
    "ddred900",
    "ddrem1000",
    "ddrem1001",
    "ddrmn1000",
    "ddrmn1001",
    "ddsub9990",
    "ddsub9991",
    "ddintx074",
    "ddintx094",
    "divx9998",
    "divx9999",
    "dvix900",
    "dvix901",
    "dqabs900",
    "dqadd9990",
    "dqadd9991",
    "dqcom990",
    "dqcom991",
    "dqcot9990",
    "dqcot9991",
    "dqctm9990",
    "dqctm9991",
    "dqdiv9998",
    "dqdiv9999",
    "dqdvi900",
    "dqdvi901",
    "dqfma2990",
    "dqfma2991",
    "dqadd39990",
    "dqadd39991",
    "dqlogb900",
    "dqmax900",
    "dqmax901",
    "dqmxg900",
    "dqmxg901",
    "dqmin900",
    "dqmin901",
    "dqmng900",
    "dqmng901",
    "dqmul9990",
    "dqmul9991",
    "dqnextm900",
    "dqnextp900",
    "dqnextt900",
    "dqnextt901",
    "dqqua998",
    "dqqua999",
    "dqred900",
    "dqrem1000",
    "dqrem1001",
    "dqrmn1000",
    "dqrmn1001",
    "dqsub9990",
    "dqsub9991",
    "dqintx074",
    "dqintx094",
    "expx900",
    "fmax2990",
    "fmax2991",
    "fmax39990",
    "fmax39991",
    "lnx900",
    "logx900",
    "logbx900",
    "maxx900",
    "maxx901",
    "mxgx900",
    "mxgx901",
    "mnm900",
    "mnm901",
    "mng900",
    "mng901",
    "minx900",
    "mulx990",
    "mulx991",
    "nextm900",
    "nextp900",
    "nextt900",
    "nextt901",
    "plu900",
    "powx900",
    "powx901",
    "pwsx900",
    "quax1022",
    "quax1023",
    "quax1024",
    "quax1025",
    "quax1026",
    "quax1027",
    "quax1028",
    "quax1029",
    "quax0a2",
    "quax0a3",
    "quax998",
    "quax999",
    "redx900",
    "remx1000",
    "remx1001",
    "rmnx900",
    "rmnx901",
    "sqtx9900",
    "subx9990",
    "subx9991",
    /* operand range violations, invalid context */
    "expx901",
    "expx902",
    "expx903",
    "expx905",
    "lnx901",
    "lnx902",
    "lnx903",
    "lnx905",
    "logx901",
    "logx902",
    "logx903",
    "logx905",
    "powx1183",
    "powx1184",
    "powx4001",
    "powx4002",
    "powx4003",
    "powx4005",
    "powx4008",
    "powx4010",
    "powx4012",
    "powx4014",
    "scbx164",
    "scbx165",
    "scbx166",
    /* skipped for decNumber, too */
    "powx4302",
    "powx4303",
    "powx4303",
    "powx4342",
    "powx4343",
    "pwsx805",
    /* disagreement for three arg power */
    "pwmx325",
    "pwmx326",
};

mpd_ssize_t
strtossize(const char *s, char **end, int base)
{
    int64_t retval;

    errno = 0;
    retval = _mpd_strtossize(s, end, base);
    if (errno == 0 && (retval > MPD_SSIZE_MAX || retval < MPD_SSIZE_MIN)) {
        errno = ERANGE;
    }
    if (errno == ERANGE) {
        return (retval < 0) ? MPD_SSIZE_MIN : MPD_SSIZE_MAX;
    }

    return (mpd_ssize_t)retval;
}

void
mpd_init_rand(Decimal &x)
{
    Context maxcontext{readcontext};
    long r = random() % 100;
    uint8_t sign = random() % 2;

    if (r >= 80) {
        x = Decimal("-1111111111e20200", maxcontext);
    }
    else if (r >= 60) {
        x = Decimal("-1111111111222222222233333333334444444444555555555566666666667777777777"
                    "888888888899999999990000000000e-1201", maxcontext);
    }
    else if (r >= 40) {
        x = sign ? Decimal("-nan") : Decimal("nan");
    }
    else if (r >= 20) {
        x = sign ? Decimal("-snan") : Decimal("snan");
    }
    else {
        x = sign ? Decimal("-inf") : Decimal("inf");
    }
}

bool
skip_test(const std::string& id)
{
    const auto& loc = std::find(std::begin(skipit), std::end(skipit), id);
    if (loc != std::end(skipit)) {
        return true;
    }

    return false;
}

bool
startswith(const std::string& s, const char *prefix)
{
    return strncasecmp(s.c_str(), prefix, strlen(prefix)) == 0;
}

bool
endswith(const std::string& s, const char *suffix)
{
    std::string rs(s);
    std::string prefix(suffix);
    std::reverse(rs.begin(), rs.end());
    std::reverse(prefix.begin(), prefix.end());
    return startswith(rs, prefix.c_str());
}

bool
eqtoken(const std::string& tok, const char *s)
{
    return strcasecmp(tok.c_str(), s) == 0;
}

bool
istokchar(unsigned char c)
{
    return std::isalnum(c) || (std::ispunct(c) && c != '"' && c != '\'');
}

int
nexttoken(std::string::const_iterator& start,
          std::string::const_iterator& end,
          std::string::const_iterator& next_start,
          const std::string::const_iterator& nul)
{
    end = next_start;

    for (; end != nul; end++) {
        if (isspace((unsigned char)*end)) {
            ;
        }
        else if (*end == '-' && (end+1) != nul && *(end+1) == '-') {
            start = end = next_start = nul;
            return 0;
        }
        else if (*end == '"') {
            start = ++end;
            for (; end != nul; end++) {
                if (*end == '"') {
                    if ((end+1) != nul && *(end+1) == '"') {
                        end++;  /* official test cases: "1""1" is parsed as a single string. */
                    }
                    else {
                        next_start = end+1;
                        return 0;
                    }
                }
            }
            return -1;
        }
        else if (*end == '\'') {
            start = ++end;
            for (; end != nul; end++) {
                if (*end == '\'') {
                    if ((end+1) != nul && *(end+1) == '\'') {
                        end++;  /* official test cases: '1''1' is parsed as a single string. */
                    }
                    else {
                        next_start = end+1;
                        return 0;
                    }
                }
            }
            return -1;
        }
        else {
            start = end;
            for (; end != nul; end++) {
                if (std::isspace((unsigned char)*end)) {
                    break;
                }
                if (!istokchar(*end)) {
                    return -1;
                }
            }
            next_start = end;
            return 0;
        }
    }

    start = next_start = end;
    return 0;
}

/* split a line into tokens */
std::vector<std::string>
split(const std::string& line)
{
    std::string::const_iterator start = line.begin();
    std::string::const_iterator end = start;
    std::string::const_iterator next_start = start;
    const std::string::const_iterator nul = line.end();
    std::vector<std::string> token;

    while (true) {
        const int r = nexttoken(start, end, next_start, nul);
        if (r < 0) {
            std::cerr << "parse_error: " << line << std::endl;
            std::exit(EXIT_FAILURE);
        }
        if (end == start && end == next_start) {
            break;
        }
        std::string tok{start, end};
        token.push_back(tok);
    }

    return token;
}

/* returns all expected conditions in a status flag */
uint32_t
scan_conditions(const std::vector<std::string>& token, const int n)
{
    uint32_t status = 0;

    for (size_t i = n; i < token.size(); i++) {
        const std::string condition = token[i];

        if (eqtoken(condition, "Clamped")) {
            status |= MPD_Clamped;
        }
        else if (eqtoken(condition, "Conversion_syntax")) {
            status |= MPD_Conversion_syntax;
        }
        else if (eqtoken(condition, "Division_by_zero")) {
            status |= MPD_Division_by_zero;
        }
        else if (eqtoken(condition, "Division_impossible")) {
            status |= MPD_Division_impossible;
        }
        else if (eqtoken(condition, "Division_undefined")) {
            status |= MPD_Division_undefined;
        }
        else if (eqtoken(condition, "Fpu_error")) {
            status |= MPD_Fpu_error;
        }
        else if (eqtoken(condition, "Inexact")) {
            status |= MPD_Inexact;
        }
        else if (eqtoken(condition, "Invalid_context")) {
            status |= MPD_Invalid_context;
        }
        else if (eqtoken(condition, "Invalid_operation")) {
            status |= MPD_Invalid_operation;
        }
        else if (eqtoken(condition, "Malloc_error")) {
            status |= MPD_Malloc_error;
        }
        else if (eqtoken(condition, "Not_implemented")) {
            status |= MPD_Not_implemented;
        }
        else if (eqtoken(condition, "Overflow")) {
            status |= MPD_Overflow;
        }
        else if (eqtoken(condition, "Rounded")) {
            status |= MPD_Rounded;
        }
        else if (eqtoken(condition, "Subnormal")) {
            status |= MPD_Subnormal;
        }
        else if (eqtoken(condition, "Underflow")) {
            status |= MPD_Underflow;
        }
        else {
            err_token(token, "scan_conditions: unknown status");
        }
    }

    return status;
}

void
compare_expected(const std::vector<std::string>& token,
                 const std::string& calc,
                 const std::string& expected,
                 uint32_t expected_status,
                 const Context& ctx)
{
    const std::string id = token.at(0);

    /* known ULP diffs */
    if (ctx.allcr() == 0) {
        for (const auto& c : ulp_cases) {
            if (id == c.id && expected == c.expected && calc == c.calc) {
                return;
            }
        }
    }

    /* known status diffs */
    for (const auto& c : status_cases) {
        if (id == c.id && expected_status == c.expected && ctx.status() == c.calc) {
            return;
        }
    }

    if (calc != expected) {
        err_token(token, "calc: ", calc, " expected: ", expected);
    }

    if (ctx.status() != expected_status) {
        char ctxstatus[MPD_MAX_FLAG_STRING];
        char expstatus[MPD_MAX_FLAG_STRING];

        mpd_snprint_flags(ctxstatus, MPD_MAX_FLAG_STRING, ctx.status()),
        mpd_snprint_flags(expstatus, MPD_MAX_FLAG_STRING, expected_status);

        err_token(token, "calc: [", ctxstatus, "] expected: [",  expstatus, "]");
    }
}

bool
equalmem(const Decimal& x, const Decimal& y)
{
    const mpd_t *a = x.getconst();
    const mpd_t *b = y.getconst();

    if ((a->flags & ~MPD_DATAFLAGS) != (b->flags & ~MPD_DATAFLAGS) ||
        a->exp != b->exp ||
        a->len != b->len ||
        a->digits != b->digits) {
        return false;
    }

    for (mpd_ssize_t i = 0; i < a->len; i++) {
        if (a->data[i] != b->data[i]) {
            return false;
        }
    }

    return true;
}

void
check_equalmem(const std::vector<std::string>& token, const Decimal& a, const Decimal& b)
{
    if (!equalmem(a, b)) {
        err_token(token, "const arg changed");
    }
}

unsigned long
get_testno(const std::vector<std::string>& token)
{
    const char *number = strpbrk(token.at(0).c_str(), "0123456789");
    if (number == NULL) {
        err_token(token, "invalid test id: ", token.at(0));
    }
    return strtoul(number, NULL, 10);
}

/* scan a single operand and the expected result */
int
scan_op_expected(Decimal& op,
                 std::string& expected,
                 const std::vector<std::string>& token,
                 Context& ctx)
{
    op = Decimal(token.at(2), ctx);
    if (token.at(3) != "->") {
        err_token(token, "expected '->' token");
    }
    expected = token.at(4);
    return 5;
}

/* scan decimal operand, string operand and the expected result */
int
scan_op_string_expected(Decimal& op1,
                        std::string& op2,
                        std::string& result,
                        const std::vector<std::string>& token,
                        Context& ctx)
{
    op1 = Decimal(token.at(2), ctx);
    op2 = token.at(3);
    if (token.at(4) != "->") {
        err_token(token, "expected '->' token");
    }
    result = token.at(5);
    return 6;
}

/* scan two operands and the expected result */
int
scan_op_op_expected(Decimal& op1,
                    Decimal& op2,
                    std::string& result,
                    const std::vector<std::string>& token,
                    Context& ctx)
{
    op1 = Decimal(token.at(2), ctx);
    op2 = Decimal(token.at(3), ctx);
    if (token.at(4) != "->") {
        err_token(token, "expected '->' token");
    }
    result = token.at(5);
    return 6;
}

/* scan one operands and two results */
int
scan_op_expected_expected(Decimal& op1,
                          std::string& result1,
                          std::string& result2,
                          const std::vector<std::string>& token,
                          Context& ctx)
{
    op1 = Decimal(token.at(2), ctx);
    if (token.at(3) != "->") {
        err_token(token, "expected '->' token");
    }
    result1 = token.at(4);
    result2 = token.at(5);
    return 6;
}

/* scan two operands and two results */
int
scan_op_op_expected_expected(Decimal& op1,
                             Decimal& op2,
                             std::string& result1,
                             std::string& result2,
                             const std::vector<std::string>& token,
                             Context& ctx)
{
    op1 = Decimal(token.at(2), ctx);
    op2 = Decimal(token.at(3), ctx);
    if (token.at(4) != "->") {
        err_token(token, "expected '->' token");
    }
    result1 = token.at(5);
    result2 = token.at(6);
    return 7;
}

/* scan three operands and the expected result */
int
scan_op_op_op_expected(Decimal& op1,
                       Decimal& op2,
                       Decimal& op3,
                       std::string& result,
                       const std::vector<std::string>& token,
                       Context& ctx)
{
    op1 = Decimal(token.at(2), ctx);
    op2 = Decimal(token.at(3), ctx);
    op3 = Decimal(token.at(4), ctx);
    if (token.at(5) != "->") {
        err_token(token, "expected '->' token");
    }
    result = token.at(6);
    return 7;
}

/*
 * This function is used for "toSci", "toEng" and "apply" and does not use
 * a maxcontext for the conversion of the operand.
 */
typedef std::string (Decimal::*String_DecimalContext)(bool) const;
void
Str_DecCtx(const std::vector<std::string>& token,
           String_DecimalContext func,
           const bool generated=false)
{
    Context maxcontext{readcontext};
    Decimal op;
    Decimal tmp;
    std::string expected;
    std::string expected_fail;
    std::string calc;

    /* status should be set in conversion (unless we have generated tests from stdin) */
    Context& workctx = generated ? maxcontext : context;
    workctx.status(0);
    const int i = scan_op_expected(op, expected, token, workctx);
    const uint32_t expstatus = scan_conditions(token, i);
    if (expstatus != workctx.status()) {
        err_token(token, "op: ", op, " expstatus: ", expstatus, " got: ", workctx.status());
    }

    /* Allocation failures for Decimal() */
    for (uint64_t n = 1; n < UINT64_MAX-1; n++) {
        mpd_init_rand(tmp);
        const Decimal save_tmp = tmp;

        workctx.status(0);
        set_alloc_fail(workctx, n);
        try {
            (void)scan_op_expected(tmp, expected_fail, token, workctx);
        }
        catch (MallocError&) {
            set_alloc(workctx);
            check_equalmem(token, tmp, save_tmp);
            continue;
        }

        set_alloc(workctx);
        break;
    }
    /* internal sanity checks */
    DECIMAL_ASSERT(expected == expected_fail, token);
    DECIMAL_ASSERT(tmp.cmp_total(op) == 0, token);

    /* make a copy of the operand */
    mpd_init_rand(tmp);
    tmp = op;

    workctx.status(0);
    calc = (tmp.*func)(true);

    /* compare the calculated result with the expected result */
    compare_expected(token, calc, expected, 0, workctx);
    check_equalmem(token, tmp, op);

    /* Allocation failures */
    for (uint64_t n = 1; n < UINT64_MAX-1; n++) {
        mpd_init_rand(tmp);
        tmp = op;

        workctx.status(0);
        set_alloc_fail(workctx, n);
        try {
            calc = (tmp.*func)(true);
        }
        catch (MallocError&) {
            set_alloc(context);
            check_equalmem(token, tmp, op);
            continue;
        }

        set_alloc(workctx);
        break;
    }

    compare_expected(token, calc, expected, 0, workctx);
    check_equalmem(token, tmp, op);
}

/* Quick and dirty: parse hex escape sequences as printed in bytestring
 * output of Python3x. */
std::string
parse_escapes_backslash(const char *s)
{
    char hex[5];
    char *result, *cp;
    unsigned int u;
    int n;

    std::shared_ptr<char> ptr(new char[strlen(s)+1], std::default_delete<char[]>());
    cp = result = ptr.get();

    hex[0] = '0';
    hex[1] = '\0';
    while (*s) {
        if (*s == '\\' && *(s+1) == 'x') {
            for (n = 1; n < 4; n++) {
                if (!s[n]) {
                    err_raise("parse hex escapes: invalid escape sequence");
                }
                hex[n] = s[n];
            }
            hex[n] = '\0';
            sscanf(hex, "%x%n", &u, &n);
            *cp++ = static_cast<char>(u);
            s += n;
        }
        else {
            *cp++ = *s++;
        }
    }

    *cp = '\0';
    return std::string(result);
}

std::string
parse_escapes_hexstring(const char *s)
{
    const std::string hex{s};
    const size_t len = hex.size();
    std::vector<char> bytes;

    if (len % 2 != 0) {
        err_raise("parse hex escapes: invalid escape sequence");
    }

    for (size_t i = 0; i < len; i += 2) {
        std::string twodigits = hex.substr(i, 2);
        unsigned char b = (unsigned char)strtoul(twodigits.c_str(), NULL, 16);
        bytes.push_back((char)b);
    }

    return std::string(bytes.data(), bytes.size());
}

std::string
parse_escapes(const char *s)
{
    if (startswith(s, "HEX")) {
        return parse_escapes_hexstring(s+3);
    }
    else {
        return parse_escapes_backslash(s);
    }
}

/* This function is used for Decimal::format. */
void
Fmt(const std::vector<std::string>& token)
{
    Context maxcontext{readcontext};
    Decimal op, tmp;
    std::string fmt, expected;
    std::string calc;

    const int i = scan_op_string_expected(op, fmt, expected, token, maxcontext);
    const uint32_t expstatus = scan_conditions(token, i);

    fmt = parse_escapes(fmt.c_str());
    expected = parse_escapes(expected.c_str());

    mpd_init_rand(tmp);
    tmp = op;

    context.status(0);
    try {
        calc = tmp.format(fmt);
    }
    catch (ValueError&) {
        DECIMAL_ASSERT(expstatus == MPD_Invalid_operation, token);
        DECIMAL_ASSERT(context.status() == 0, token);
        check_equalmem(token, tmp, op);
    }

    DECIMAL_ASSERT(expstatus == 0 || expstatus == MPD_Invalid_operation, token);
    if (expstatus == 0) {
        compare_expected(token, calc, expected, expstatus, context);
        check_equalmem(token, tmp, op);
    }

    for (uint64_t n = 1; n < UINT64_MAX-1; n++) {
        mpd_init_rand(tmp);
        tmp = op;

        context.status(0);
        set_alloc_fail(context, n);
        try {
            calc = tmp.format(fmt);
        }
        catch (MallocError&) {
            set_alloc(context);
            continue;
        }
        catch (ValueError&) {
            DECIMAL_ASSERT(expstatus == MPD_Invalid_operation, token);
            DECIMAL_ASSERT(context.status() == 0, token);
        }

        set_alloc(context);
        break;
    }

    DECIMAL_ASSERT(expstatus == 0 || expstatus == MPD_Invalid_operation, token);
    if (expstatus == 0) {
        compare_expected(token, calc, expected, expstatus, context);
        check_equalmem(token, tmp, op);
    }
}

void
Class(const std::vector<std::string>& token)
{
    Context maxcontext{readcontext};
    Decimal op, tmp;
    std::string expected;

    const int n = scan_op_expected(op, expected, token, maxcontext);
    const uint32_t expstatus = scan_conditions(token, n);

    mpd_init_rand(tmp);
    tmp = op;

    context.status(0);
    std::string calc = tmp.number_class(context);
    compare_expected(token, calc, expected, expstatus, context);
    check_equalmem(token, tmp, op);
}

/* test a unary function */
typedef Decimal (Decimal::*Decimal_Decimal)() const;

void
Dec_Dec_RunSingle(Decimal& result, Decimal& tmp,
                  const std::vector<std::string>& token,
                  const Decimal_Decimal func,
                  const Decimal& op,
                  const std::string& expected,
                  const uint32_t expstatus)
{
    int incr = 1;
    for (uint64_t n = 1; n < UINT64_MAX-100; n += incr) {
        mpd_init_rand(result);
        mpd_init_rand(tmp);
        tmp = op;

        const Decimal save_result = result;
        context.status(0);
        set_alloc_fail(context, n);
        try {
            result = (tmp.*func)();
        }
        catch (MallocError&) {
            set_alloc(context);
            check_equalmem(token, result, save_result);
            check_equalmem(token, tmp, op);
            if (n > 50) {
                incr = random() % 100 + 1;
            }
            continue;
        }

        set_alloc(context);
        break;
    }

    const std::string calc = result.to_sci();
    compare_expected(token, calc, expected, expstatus, context);
    if (&tmp != &result) {
        check_equalmem(token, tmp, op);
    }
}

void
Dec_Dec(const std::vector<std::string>& token, Decimal_Decimal func)
{
    Context maxcontext{readcontext};
    Decimal op, result, tmp;
    std::string expected;

    const int n = scan_op_expected(op, expected, token, maxcontext);
    const uint32_t expstatus = scan_conditions(token, n);

    Dec_Dec_RunSingle(result, tmp, token, func, op, expected, expstatus);
    Dec_Dec_RunSingle(tmp, tmp, token, func, op, expected, expstatus);
}

/* test a unary function with an optional context argument */
typedef Decimal (Decimal::*Decimal_DecimalContext)(Context&) const;

void
Dec_DecCtx_RunSingle(Decimal& result, Decimal& tmp,
                     const std::vector<std::string>& token,
                     const Decimal_DecimalContext func,
                     const Decimal& op,
                     const std::string& expected,
                     const uint32_t expstatus)
{
    int incr = 1;
    for (uint64_t n = 1; n < UINT64_MAX-100; n += incr) {
        mpd_init_rand(result);
        mpd_init_rand(tmp);
        tmp = op;

        const Decimal save_result = result;
        context.status(0);
        set_alloc_fail(context, n);
        try {
            result = (tmp.*func)(context);
        }
        catch (MallocError&) {
            set_alloc(context);
            check_equalmem(token, result, save_result);
            check_equalmem(token, tmp, op);
            if (n > 50) {
                incr = random() % 100 + 1;
            }
            continue;
        }

        set_alloc(context);
        break;
    }

    const std::string calc = result.to_sci();
    compare_expected(token, calc, expected, expstatus, context);
    if (&tmp != &result) {
        check_equalmem(token, tmp, op);
    }
}

void
Dec_DecCtx(const std::vector<std::string>& token, Decimal_DecimalContext func)
{
    Context maxcontext{readcontext};
    Decimal op, result, tmp;
    std::string expected;

    const int n = scan_op_expected(op, expected, token, maxcontext);
    const uint32_t expstatus = scan_conditions(token, n);

    Dec_DecCtx_RunSingle(result, tmp, token, func, op, expected, expstatus);
    Dec_DecCtx_RunSingle(tmp, tmp, token, func, op, expected, expstatus);
}

/* Same as Dec_DecCtx, but quantize the operand before applying the actual function */
void
Dec_DecCtxWithQuantize(const std::vector<std::string>& token, Decimal_DecimalContext func)
{
    Context maxcontext{readcontext};
    Decimal op, scale, result, tmp;
    std::string expected;

    const int n = scan_op_op_expected(op, scale, expected, token, maxcontext);
    const uint32_t expstatus = scan_conditions(token, n);

    op = op.quantize(scale, maxcontext);

    Dec_DecCtx_RunSingle(result, tmp, token, func, op, expected, expstatus);
    Dec_DecCtx_RunSingle(tmp, tmp, token, func, op, expected, expstatus);
}

/* Test a binary function */
typedef Decimal (Decimal::*Decimal_DecimalDecimalContext)(const Decimal&, Context&) const;

void
resolve_status_hack(uint32_t& expstatus, const uint32_t status)
{
    /* hack #1 to resolve disagreement with results generated by decimal.py */
    if ((expstatus & MPD_Invalid_operation) &&
        (status & MPD_Division_impossible)) {
        expstatus = MPD_Division_impossible;
    }

    /* hack #2 to resolve disagreement with results generated by decimal.py */
    if ((expstatus & MPD_Invalid_operation) &&
        (status & MPD_Division_undefined)) {
        expstatus = MPD_Division_undefined;
    }
}

void
Dec_DecDecCtx_RunSingle(Decimal& result, Decimal& tmp1, Decimal& tmp2,
                        const std::vector<std::string>& token,
                        const Decimal_DecimalDecimalContext func,
                        const Decimal& op1, const Decimal &op2,
                        const std::string& expected,
                        const uint32_t expstatus)
{
    int incr = 1;
    for (uint64_t n = 1; n < UINT64_MAX-100; n += incr) {
        mpd_init_rand(tmp1);
        mpd_init_rand(tmp2);
        tmp1 = op1;
        tmp2 = op2;

        const Decimal save_result = result;
        context.status(0);
        set_alloc_fail(context, n);
        try {
            result = (tmp1.*func)(tmp2, context);
        }
        catch (MallocError&) {
            set_alloc(context);
            check_equalmem(token, result, save_result);
            check_equalmem(token, tmp1, op1);
            check_equalmem(token, tmp2, op2);
            if (n > 50) {
               incr = random() % 100 + 1;
            }
            continue;
        }

        set_alloc(context);
        break;
    }

    const std::string calc = result.to_sci();
    compare_expected(token, calc, expected, expstatus, context);
    if (&tmp1 != &result) {
        check_equalmem(token, tmp1, op1);
    }
    if (&tmp2 != &result) {
        check_equalmem(token, tmp2, op2);
    }
}

void
Dec_DecDecCtx(const std::vector<std::string>& token,
              const Decimal_DecimalDecimalContext func,
              bool scan_equal=false,
              bool resolve_status=false)
{
    Context maxcontext{readcontext};
    Decimal result, tmp1, tmp2;
    Decimal op1, op2;
    std::string expected;
    uint32_t expstatus;
    int n;

    if (scan_equal) {
        n = scan_op_expected(op1, expected, token, maxcontext);
        op2 = op1;
    }
    else {
        n = scan_op_op_expected(op1, op2, expected, token, maxcontext);
    }
    expstatus = scan_conditions(token, n);

    context.status(0);
    result = (op1.*func)(op2, context);
    if (resolve_status) {
        resolve_status_hack(expstatus, context.status());
    }

    Dec_DecDecCtx_RunSingle(result, tmp1, tmp2, token, func, op1, op2, expected, expstatus);
    Dec_DecDecCtx_RunSingle(tmp1, tmp1, tmp2, token, func, op1, op2, expected, expstatus);
    Dec_DecDecCtx_RunSingle(tmp2, tmp1, tmp2, token, func, op1, op2, expected, expstatus);

    if (equalmem(op1, op2)) {
        Dec_DecDecCtx_RunSingle(result, tmp1, tmp1, token, func, op1, op2, expected, expstatus);
        Dec_DecDecCtx_RunSingle(tmp1, tmp1, tmp1, token, func, op1, op2, expected, expstatus);
    }
}

/* Test a binary function with a binary result */
typedef std::pair<Decimal, Decimal> (Decimal::*DecimalPair_DecimalDecimalContext)(const Decimal&, Context&) const;

void
DecPair_DecDecCtx_RunSingle(std::pair<Decimal, Decimal>& result, Decimal& tmp1, Decimal& tmp2,
                            const std::vector<std::string>& token,
                            const DecimalPair_DecimalDecimalContext func,
                            const Decimal& op1, const Decimal &op2,
                            const std::string& expected1, const std::string& expected2,
                            const uint32_t expstatus)
{
    int incr = 1;
    for (uint64_t n = 1; n < UINT64_MAX-100; n += incr) {
        mpd_init_rand(tmp1);
        mpd_init_rand(tmp2);
        tmp1 = op1;
        tmp2 = op2;
        const Decimal first = result.first;
        const Decimal second = result.second;

        context.status(0);
        set_alloc_fail(context, n);
        try {
            result = (tmp1.*func)(tmp2, context);
        }
        catch (MallocError&) {
            set_alloc(context);
            check_equalmem(token, result.first, first);
            check_equalmem(token, result.second, second);
            check_equalmem(token, tmp1, op1);
            check_equalmem(token, tmp2, op2);
            if (n > 50) {
                incr = random() % 100 + 1;
            }
            continue;
        }

        set_alloc(context);
        break;
    }

    std::string calc = result.first.to_sci();
    compare_expected(token, calc, expected1, expstatus, context);

    calc = result.second.to_sci();
    compare_expected(token, calc, expected2, expstatus, context);

    if (&tmp1 != &result.first && &tmp1 != &result.second) {
        check_equalmem(token, tmp1, op1);
    }
    if (&tmp2 != &result.first && &tmp2 != &result.second) {
        check_equalmem(token, tmp2, op2);
    }
}

void
DecPair_DecDecCtx(const std::vector<std::string>& token,
                  const DecimalPair_DecimalDecimalContext func,
                  bool scan_equal=false)
{
    Context maxcontext{readcontext};
    std::pair<Decimal, Decimal> result;
    Decimal tmp1, tmp2;
    Decimal op1, op2;
    std::string expected1, expected2;
    uint32_t expstatus;
    int n;

    if (scan_equal) {
        n = scan_op_expected_expected(op1, expected1, expected2,
                                      token, maxcontext);
        op2 = op1;
    }
    else {
        n = scan_op_op_expected_expected(op1, op2, expected1, expected2,
                                         token, maxcontext);
    }
    expstatus = scan_conditions(token, n);

    context.status(0);
    result = (op1.*func)(op2, context);
    resolve_status_hack(expstatus, context.status());

    DecPair_DecDecCtx_RunSingle(result, tmp1, tmp2, token, func, op1, op2, expected1, expected2, expstatus);
    DecPair_DecDecCtx_RunSingle(result, result.first, tmp2, token, func, op1, op2, expected1, expected2, expstatus);
    DecPair_DecDecCtx_RunSingle(result, tmp1, result.first, token, func, op1, op2, expected1, expected2, expstatus);
    DecPair_DecDecCtx_RunSingle(result, result.second, tmp2, token, func, op1, op2, expected1, expected2, expstatus);
    DecPair_DecDecCtx_RunSingle(result, tmp1, result.second, token, func, op1, op2, expected1, expected2, expstatus);

    if (equalmem(op1, op2)) {
        DecPair_DecDecCtx_RunSingle(result, tmp1, tmp1, token, func, op1, op2, expected1, expected2, expstatus);
        DecPair_DecDecCtx_RunSingle(result, result.first, result.first, token, func, op1, op2, expected1, expected2, expstatus);
        DecPair_DecDecCtx_RunSingle(result, result.second, result.second, token, func, op1, op2, expected1, expected2, expstatus);
    }
}

/* Test a ternary function */
typedef Decimal (Decimal::*Decimal_DecimalDecimalDecimalContext)(const Decimal&, const Decimal&, Context&) const;

void
Dec_DecDecDecCtx_RunSingle(Decimal& result, Decimal& tmp1, Decimal& tmp2, Decimal& tmp3,
                           const std::vector<std::string>& token,
                           const Decimal_DecimalDecimalDecimalContext func,
                           const Decimal& op1, const Decimal &op2, const Decimal &op3,
                           const std::string& expected,
                           const uint32_t expstatus)
{
    int incr = 1;
    for (uint64_t n = 1; n < UINT64_MAX-100; n += incr) {
        mpd_init_rand(tmp1);
        mpd_init_rand(tmp2);
        mpd_init_rand(tmp3);
        tmp1 = op1;
        tmp2 = op2;
        tmp3 = op3;

        const Decimal save_result = result;
        context.status(0);
        set_alloc_fail(context, n);
        try {
            result = (tmp1.*func)(tmp2, tmp3, context);
        }
        catch (MallocError&) {
            set_alloc(context);
            check_equalmem(token, result, save_result);
            check_equalmem(token, tmp1, op1);
            check_equalmem(token, tmp2, op2);
            check_equalmem(token, tmp3, op3);
            if (n > 100) {
                incr = random() % 100 + 1;
            }
            continue;
        }

        set_alloc(context);
        break;
    }

    const std::string calc = result.to_sci();
    compare_expected(token, calc, expected, expstatus, context);
    if (&tmp1 != &result) {
        check_equalmem(token, tmp1, op1);
    }
    if (&tmp2 != &result) {
        check_equalmem(token, tmp2, op2);
    }
    if (&tmp3 != &result) {
        check_equalmem(token, tmp3, op3);
    }
}

enum ternary_equal { OpOpOp, EqEqOp, EqOpEq, OpEqEq, EqEqEq };
void
Dec_DecDecDecCtx(const std::vector<std::string>& token,
                 const Decimal_DecimalDecimalDecimalContext func,
                 enum ternary_equal scan_equal=OpOpOp)
{
    Context maxcontext{readcontext};
    Decimal result, tmp1, tmp2, tmp3;
    Decimal op1, op2, op3;
    std::string expected;
    uint32_t expstatus;
    int n;

    switch (scan_equal) {
    case OpOpOp:
        n = scan_op_op_op_expected(op1, op2, op3, expected, token, maxcontext);
        break;
    case EqEqOp:
        n = scan_op_op_expected(op1, op3, expected, token, maxcontext);
        op2 = op1;
        break;
    case EqOpEq:
        n = scan_op_op_expected(op1, op2, expected, token, maxcontext);
        op3 = op1;
        break;
    case OpEqEq:
        n = scan_op_op_expected(op1, op2, expected, token, maxcontext);
        op3 = op2;
        break;
    case EqEqEq:
        n = scan_op_expected(op1, expected, token, maxcontext);
        op3 = op2 = op1;
        break;
    default:
        err_raise("internal error: unexpected tag");
        break;
    }
    expstatus = scan_conditions(token, n);

    Dec_DecDecDecCtx_RunSingle(result, tmp1, tmp2, tmp3, token, func, op1, op2, op3, expected, expstatus);
    Dec_DecDecDecCtx_RunSingle(result, result, tmp2, tmp3, token, func, op1, op2, op3, expected, expstatus);
    Dec_DecDecDecCtx_RunSingle(result, tmp1, result, tmp3, token, func, op1, op2, op3, expected, expstatus);
    Dec_DecDecDecCtx_RunSingle(result, tmp1, tmp2, result, token, func, op1, op2, op3, expected, expstatus);

    if (equalmem(op1, op2)) {
        Dec_DecDecDecCtx_RunSingle(result, tmp1, tmp1, tmp3, token, func, op1, op2, op3, expected, expstatus);
        Dec_DecDecDecCtx_RunSingle(result, result, result, tmp3, token, func, op1, op2, op3, expected, expstatus);
        Dec_DecDecDecCtx_RunSingle(result, tmp1, tmp1, result, token, func, op1, op2, op3, expected, expstatus);
    }

    if (equalmem(op1, op3)) {
        Dec_DecDecDecCtx_RunSingle(result, tmp1, tmp2, tmp1, token, func, op1, op2, op3, expected, expstatus);
        Dec_DecDecDecCtx_RunSingle(result, result, tmp2, result, token, func, op1, op2, op3, expected, expstatus);
        Dec_DecDecDecCtx_RunSingle(result, tmp1, result, tmp1, token, func, op1, op2, op3, expected, expstatus);
    }

    if (equalmem(op2, op3)) {
        Dec_DecDecDecCtx_RunSingle(result, tmp1, tmp2, tmp2, token, func, op1, op2, op3, expected, expstatus);
        Dec_DecDecDecCtx_RunSingle(result, result, tmp2, tmp2, token, func, op1, op2, op3, expected, expstatus);
        Dec_DecDecDecCtx_RunSingle(result, tmp1, result, result, token, func, op1, op2, op3, expected, expstatus);
    }

    if (equalmem(op1, op2) && equalmem(op1, op3)) {
        Dec_DecDecDecCtx_RunSingle(result, tmp1, tmp1, tmp1, token, func, op1, op2, op3, expected, expstatus);
        Dec_DecDecDecCtx_RunSingle(result, result, result, result, token, func, op1, op2, op3, expected, expstatus);
    }
}

/* Test a binary function with no context argument */
typedef Decimal (Decimal::*Decimal_DecimalDecimal)(const Decimal&) const;

void
Dec_DecDec_RunSingle(Decimal& result, Decimal& tmp1, Decimal& tmp2,
                     const std::vector<std::string>& token,
                     const Decimal_DecimalDecimal func,
                     const Decimal& op1, const Decimal &op2,
                     const std::string& expected,
                     const uint32_t expstatus)
{
    for (uint64_t n = 1; n < UINT64_MAX-1; n++) {
        mpd_init_rand(tmp1);
        mpd_init_rand(tmp2);
        tmp1 = op1;
        tmp2 = op2;

        const Decimal save_result = result;
        context.status(0);
        set_alloc_fail(context, n);
        try {
            result = (tmp1.*func)(tmp2);
        }
        catch (MallocError&) {
            set_alloc(context);
            check_equalmem(token, result, save_result);
            check_equalmem(token, tmp1, op1);
            check_equalmem(token, tmp2, op2);
            continue;
        }

        set_alloc(context);
        break;
    }

    const std::string calc = result.to_sci();
    compare_expected(token, calc, expected, expstatus, context);
    if (&tmp1 != &result) {
        check_equalmem(token, tmp1, op1);
    }
    if (&tmp2 != &result) {
        check_equalmem(token, tmp2, op2);
    }
}

void
Dec_DecDec(const std::vector<std::string>& token,
           const Decimal_DecimalDecimal func,
           bool scan_equal=false,
           bool resolve_status=false)
{
    Context maxcontext{readcontext};
    Decimal result, tmp1, tmp2;
    Decimal op1, op2;
    std::string expected;
    uint32_t expstatus;
    int n;

    if (scan_equal) {
        n = scan_op_expected(op1, expected, token, maxcontext);
        op2 = op1;
    }
    else {
        n = scan_op_op_expected(op1, op2, expected, token, maxcontext);
    }
    expstatus = scan_conditions(token, n);

    if (resolve_status) {
        resolve_status_hack(expstatus, context.status());
    }

    Dec_DecDec_RunSingle(result, tmp1, tmp2, token, func, op1, op2, expected, expstatus);
    Dec_DecDec_RunSingle(tmp1, tmp1, tmp2, token, func, op1, op2, expected, expstatus);
    Dec_DecDec_RunSingle(tmp2, tmp1, tmp2, token, func, op1, op2, expected, expstatus);

    if (equalmem(op1, op2)) {
        Dec_DecDec_RunSingle(result, tmp1, tmp1, token, func, op1, op2, expected, expstatus);
        Dec_DecDec_RunSingle(tmp1, tmp1, tmp1, token, func, op1, op2, expected, expstatus);
    }
}

/* Test a binary function that returns an integer result */
typedef int (Decimal::*Int_DecimalDecimal)(const Decimal&) const;

void
Int_DecDec_RunSingle(Decimal& tmp1, Decimal& tmp2,
                     const enum skip_cmp skip,
                     const std::vector<std::string>& token,
                     const Int_DecimalDecimal func,
                     const Decimal& op1, const Decimal &op2,
                     const std::string& expected,
                     const uint32_t expstatus)
{
    int int_result = -101;

    for (uint64_t n = 1; n < UINT64_MAX-1; n++) {
        mpd_init_rand(tmp1);
        mpd_init_rand(tmp2);
        tmp1 = op1;
        tmp2 = op2;

        context.status(0);
        set_alloc_fail(context, n);
        try {
            int_result = (tmp1.*func)(tmp2);
        }
        catch (MallocError&) {
            set_alloc(context);
            check_equalmem(token, tmp1, op1);
            check_equalmem(token, tmp2, op2);
            continue;
        }

        set_alloc(context);
        break;
    }

    char buf[11];
    snprintf(buf, sizeof buf, "%d", int_result);
    if (skip == SKIP_NONE || int_result != INT_MAX) {
        compare_expected(token, buf, expected, expstatus, context);
    }
    check_equalmem(token, tmp1, op1);
    check_equalmem(token, tmp2, op2);
}

void
Int_DecDec(const enum skip_cmp skip,
           const std::vector<std::string>& token,
           const Int_DecimalDecimal func,
           bool scan_equal=false)
{
    Context maxcontext{readcontext};
    Decimal tmp1, tmp2;
    Decimal op1, op2;
    std::string expected;
    uint32_t expstatus;
    int n;

    if (scan_equal) {
        n = scan_op_expected(op1, expected, token, maxcontext);
        op2 = op1;
    }
    else {
        n = scan_op_op_expected(op1, op2, expected, token, maxcontext);
    }
    expstatus = scan_conditions(token, n);

    Int_DecDec_RunSingle(tmp1, tmp2, skip, token, func, op1, op2, expected, expstatus);
    if (equalmem(op1, op2)) {
        Int_DecDec_RunSingle(tmp1, tmp1, skip, token, func, op1, op2, expected, expstatus);
    }
}

/* Test a binary function that returns a bool result */
typedef bool (Decimal::*Bool_DecimalDecimal)(const Decimal&) const;

void
Bool_DecDec_RunSingle(Decimal& tmp1, Decimal& tmp2,
                      const enum skip_cmp skip,
                      const std::vector<std::string>& token,
                      const Bool_DecimalDecimal func,
                      const Decimal& op1, const Decimal &op2,
                      const std::string& expected,
                      const uint32_t expstatus)
{
    int int_result = -101;

    for (uint64_t n = 1; n < UINT64_MAX-1; n++) {
        mpd_init_rand(tmp1);
        mpd_init_rand(tmp2);
        tmp1 = op1;
        tmp2 = op2;

        context.status(0);
        set_alloc_fail(context, n);
        try {
            int_result = (tmp1.*func)(tmp2);
        }
        catch (MallocError&) {
            set_alloc(context);
            DECIMAL_ASSERT(int_result== INT_MAX, token);
            check_equalmem(token, tmp1, op1);
            check_equalmem(token, tmp2, op2);
            continue;
        }

        set_alloc(context);
        break;
    }

    char buf[11];
    snprintf(buf, 11, "%d", int_result);
    if (skip == SKIP_NONE || int_result != INT_MAX) {
        compare_expected(token, buf, expected, expstatus, context);
    }
    check_equalmem(token, tmp1, op1);
    check_equalmem(token, tmp2, op2);
}

void
Bool_DecDec(const enum skip_cmp skip,
            const std::vector<std::string>& token,
            const Bool_DecimalDecimal func,
            bool scan_equal=false)
{
    Context maxcontext{readcontext};
    Decimal tmp1, tmp2;
    Decimal op1, op2;
    std::string expected;
    uint32_t expstatus;
    int n;

    if (scan_equal) {
        n = scan_op_expected(op1, expected, token, maxcontext);
        op2 = op1;
    }
    else {
        n = scan_op_op_expected(op1, op2, expected, token, maxcontext);
    }
    expstatus = scan_conditions(token, n);

    Bool_DecDec_RunSingle(tmp1, tmp2, skip, token, func, op1, op2, expected, expstatus);
    if (equalmem(op1, op2)) {
        Bool_DecDec_RunSingle(tmp1, tmp1, skip, token, func, op1, op2, expected, expstatus);
    }
}

mpd_ssize_t
scan_ssize(const std::string& tok)
{
    errno = 0;
    mpd_ssize_t x = strtossize(tok.c_str(), NULL, 10);
    if (errno != 0) {
        err_raise("invalid conversion to ssize_t");
    }
    return x;
}

/* Test a function with a Decimal and an int64_t operand */
typedef Decimal (Decimal::*Decimal_DecimalInt64Context)(int64_t, Context&) const;

void
Dec_DecInt64_RunSingle(Decimal& result, Decimal& tmp,
                       const std::vector<std::string>& token,
                       const Decimal_DecimalInt64Context func,
                       const Decimal& op,
                       const int64_t i64,
                       const std::string& expected,
                       const uint32_t expstatus)
{
    /* Allocation failures */
    for (uint64_t n = 1; n < UINT64_MAX-1; n++) {
        mpd_init_rand(tmp);
        tmp = op;

        context.status(0);
        set_alloc_fail(context, n);
        try {
            result = (tmp.*func)(i64, context);
        }
        catch (MallocError&) {
            check_equalmem(token, tmp, op);
            set_alloc(context);
            continue;
        }

        set_alloc(context);
        break;
    }

    const std::string calc = result.to_sci();
    compare_expected(token, calc, expected, expstatus, context);
    if (&tmp != &result) {
        check_equalmem(token, tmp, op);
    }
}

void
Dec_DecInt64Ctx(const std::vector<std::string>& token,
                const Decimal_DecimalInt64Context func)
{
    Context maxcontext{readcontext};
    Decimal result, tmp;
    Decimal op1, op2;
    std::string expected;

    const int n = scan_op_op_expected(op1, op2, expected, token, maxcontext);
    const uint32_t expstatus = scan_conditions(token, n);

    if (op2.isspecial() || op2.exponent() != 0) {
        return;
    }

    const int64_t i64 = mpd_get_ssize(op2.getconst(), maxcontext.get());
    if (maxcontext.status() & MPD_Invalid_operation) {
        return;
    }

    Dec_DecInt64_RunSingle(result, tmp, token, func, op1, i64, expected, expstatus);
    Dec_DecInt64_RunSingle(tmp, tmp, token, func, op1, i64, expected, expstatus);
}

/* Test decimal::ln10 */
void
ln10(const std::vector<std::string>& token)
{
    Context maxcontext{readcontext};
    Decimal result;
    Decimal op;
    std::string expected;

    const int n = scan_op_expected(op, expected, token, maxcontext);
    const uint32_t expstatus = scan_conditions(token, n);

    if (op.isspecial() || op.exponent() != 0) {
        return;
    }

    const int64_t i64 = mpd_get_ssize(op.getconst(), maxcontext.get());
    if (maxcontext.status() & MPD_Invalid_operation) {
        return;
    }

    for (uint64_t i = 1; i < UINT64_MAX-1; i++) {
        const Decimal save_result = result;

        context.status(0);
        set_alloc_fail(context, i);
        try {
            result = Decimal::ln10(i64, context);
        }
        catch (MallocError&) {
            set_alloc(context);
            check_equalmem(token, result, save_result);
            continue;
        }

        set_alloc(context);
        break;
    }

    const std::string calc = result.to_sci();
    compare_expected(token, calc, expected, expstatus, context);
}

/* Test u64() */
void
u64_DecCtx(const std::vector<std::string>& token)
{
    Context maxcontext{readcontext};
    Decimal op;
    uint64_t u64;
    char calc[23];
    std::string expected;

    const int n = scan_op_expected(op, expected, token, maxcontext);
    const uint32_t expstatus = scan_conditions(token, n);

    context.status(0);
    try {
        u64 = op.u64();
    }
    catch (ValueError&) {
        DECIMAL_ASSERT(expstatus == MPD_Invalid_operation, token);
        return;
    }

    snprintf(calc, 23, "%" PRIu64, u64);
    compare_expected(token, calc, expected, expstatus, context);
}

/* Test u32() */
void
u32_DecCtx(const std::vector<std::string>& token)
{
    Context maxcontext{readcontext};
    Decimal op;
    std::string expected;
    uint32_t u32;
    char calc[23];

    const int n = scan_op_expected(op, expected, token, maxcontext);
    const uint32_t expstatus = scan_conditions(token, n);

    context.status(0);
    try {
        u32 = op.u32();
    }
    catch (ValueError&) {
        DECIMAL_ASSERT(expstatus == MPD_Invalid_operation, token);
        return;
    }

    snprintf(calc, sizeof calc, "%" PRIu32, u32);
    compare_expected(token, calc, expected, 0, context);
}

/* Test a function returning an int64_t */
void
i64_DecCtx(const std::vector<std::string>& token)
{
    Context maxcontext{readcontext};
    Decimal op;
    std::string expected;
    int64_t i64;
    char calc[23];

    const int n = scan_op_expected(op, expected, token, maxcontext);
    const uint32_t expstatus = scan_conditions(token, n);

    context.status(0);
    try {
        i64 = op.i64();
    }
    catch (ValueError&) {
        DECIMAL_ASSERT(expstatus == MPD_Invalid_operation, token);
        return;
    }

    snprintf(calc, sizeof calc, "%" PRIi64, i64);
    compare_expected(token, calc, expected, 0, context);
}

/* Test a function returning an int64_t */
void
i32_DecCtx(const std::vector<std::string>& token)
{
    Context maxcontext{readcontext};
    Decimal op;
    std::string expected;
    int32_t i32;
    char calc[23];

    const int n = scan_op_expected(op, expected, token, maxcontext);
    const uint32_t expstatus = scan_conditions(token, n);

    context.status(0);
    try {
        i32 = op.i32();
    }
    catch (ValueError&) {
        DECIMAL_ASSERT(expstatus == MPD_Invalid_operation, token);
        return;
    }

    snprintf(calc, sizeof calc, "%" PRIi32, i32);
    compare_expected(token, calc, expected, 0, context);
}


/* process an input stream of test cases */
void
do_stream(std::istream& in, const bool generated=false)
{
    std::string line;

    context = Context(testcontext);

    while (std::getline(in, line)) {
        std::vector<std::string> token = split(line);
        if (token.size() == 0) {
            continue;
        }

        if (startswith(token.at(0), "Precision")) {
            if (token.at(1) == "MAX_PREC") {
                context.prec(MPD_MAX_PREC);
            }
            else {
                mpd_context_t ctx = *context.getconst();
                const mpd_ssize_t l = scan_ssize(token.at(1));
                ctx.prec = l;
                context = Context(ctx);
            }
            continue;
        }

        if (startswith(token.at(0), "MinExponent")) {
            if (token.at(1) == "MIN_EMIN") {
                context.emin(MPD_MIN_EMIN);
            }
            else {
                mpd_context_t ctx = *context.getconst();
                const mpd_ssize_t l = scan_ssize(token.at(1));
                ctx.emin = l;
                context = Context(ctx);
            }
            continue;
        }

        if (startswith(token.at(0), "MaxExponent")) {
            if (token.at(1) == "MAX_EMAX") {
                context.emax(MPD_MAX_EMAX);
            }
            else {
                mpd_context_t ctx = *context.getconst();
                const mpd_ssize_t l = scan_ssize(token.at(1));
                ctx.emax = l;
                context = Context(ctx);
            }
            continue;
        }

        if (startswith(token.at(0), "Rounding")) {
            if (eqtoken(token.at(1), "Up")) {
                context.round(MPD_ROUND_UP);
            }
            else if (eqtoken(token.at(1), "Down")) {
                context.round(MPD_ROUND_DOWN);
            }
            else if (eqtoken(token.at(1), "Ceiling")) {
                context.round(MPD_ROUND_CEILING);
            }
            else if (eqtoken(token.at(1), "Floor")) {
                context.round(MPD_ROUND_FLOOR);
            }
            else if (eqtoken(token.at(1), "Half_up")) {
                context.round(MPD_ROUND_HALF_UP);
            }
            else if (eqtoken(token.at(1), "Half_down")) {
                context.round(MPD_ROUND_HALF_DOWN);
            }
            else if (eqtoken(token.at(1), "Half_even")) {
                context.round(MPD_ROUND_HALF_EVEN);
            }
            else if (eqtoken(token.at(1), "05up")) {
                context.round(MPD_ROUND_05UP);
            }
            else {
                err_token(token, "invalid rounding mode");
            }

            continue;
        }

        if (startswith(token.at(0), "Clamp")) {
            const int l = static_cast<int>(scan_ssize(token.at(1)));
            context.clamp(l);
            continue;
        }

        if (startswith(token.at(0), "Locale")) {
            if (setlocale(LC_NUMERIC, token.at(1).c_str()) == NULL) {
                err_token(token, "invalid or missing locale");
            }
            continue;
        }

        if (startswith(token.at(0), "Version")) {
            continue;  /* optional directive */
        }

        if (startswith(token.at(0), "Extended")) {
            continue;  /* optional directive */
        }

        /*
         * Actual tests start here:
         *   - token.at(0) is the id
         *   - token.at(1) is the operation type
         *   - testno can be used for setting a watchpoint in the debugger
         */
        const unsigned long testno = get_testno(token);
        (void)testno;

        if (skip_test(token.at(0))) {
            continue;  /* id is in the skip list */
        }

#ifdef CONFIG_64
        if (startswith(token.at(0), "cov32")) {
            continue;  /* skip 32-bit specific coverage tests */
        }
#else
        if (startswith(token.at(0), "cov64")) {
            continue;  /* skip 64-bit specific coverage tests */
        }
#endif

        if (startswith(token.at(0), "pwmx")) {
            token.at(1) = std::string("powmod");
        }

        /* Unary functions with std::string result */
        if (eqtoken(token.at(1), "tosci") || eqtoken(token.at(1), "apply")) {
            Str_DecCtx(token, &Decimal::to_sci, generated);
        }
        else if (eqtoken(token.at(1), "toeng")) {
            Str_DecCtx(token, &Decimal::to_eng, generated);
        }
        else if (eqtoken(token.at(1), "format")) {
            Fmt(token);
        }

        /* Unary function with const char * result */
        else if (eqtoken(token.at(1), "class")) {
            Class(token);
        }

        /* Unary functions with Decimal result */
        else if (eqtoken(token.at(1), "abs")) {
            Dec_DecCtx(token, &Decimal::abs);
        }
        else if (eqtoken(token.at(1), "copy")) {
            Dec_Dec(token, &Decimal::copy);
        }
        else if (eqtoken(token.at(1), "copyabs")) {
            Dec_Dec(token, &Decimal::copy_abs);
        }
        else if (eqtoken(token.at(1), "copynegate")) {
            Dec_Dec(token, &Decimal::copy_negate);
        }
        else if (eqtoken(token.at(1), "exp")) {
            if (!generated) {
                if (testno != 126) {  /* 126: err < 1ulp, but not correctly rounded */
                    context.allcr(0);
                    Dec_DecCtx(token, &Decimal::exp);
                    context.allcr(1);
                }
            }
            Dec_DecCtx(token, &Decimal::exp);
        }
        else if (eqtoken(token.at(1), "invert")) {
            Dec_DecCtx(token, &Decimal::logical_invert);
        }
        else if (eqtoken(token.at(1), "invroot")) {
            Dec_DecCtx(token, &Decimal::invroot);
        }
        else if (eqtoken(token.at(1), "ln")) {
            if (!generated) {
                context.allcr(0);
                Dec_DecCtx(token, &Decimal::ln);
                context.allcr(1);
            }
            Dec_DecCtx(token, &Decimal::ln);
        }
        else if (eqtoken(token.at(1), "log10")) {
            if (!generated) {
                context.allcr(0);
                Dec_DecCtx(token,  &Decimal::log10);
                context.allcr(1);
            }
            Dec_DecCtx(token,  &Decimal::log10);
        }
        else if (eqtoken(token.at(1), "logb")) {
            Dec_DecCtx(token, &Decimal::logb);
        }
        else if (eqtoken(token.at(1), "minus")) {
            Dec_DecCtx(token, &Decimal::minus);
        }
        else if (eqtoken(token.at(1), "nextminus")) {
            Dec_DecCtx(token, &Decimal::next_minus);
        }
        else if (eqtoken(token.at(1), "nextplus")) {
            Dec_DecCtx(token, &Decimal::next_plus);
        }
        else if (eqtoken(token.at(1), "plus")) {
            Dec_DecCtx(token, &Decimal::plus);
        }
        else if (eqtoken(token.at(1), "reduce")) {
            Dec_DecCtx(token, &Decimal::reduce);
        }
        else if (eqtoken(token.at(1), "squareroot")) {
            #ifdef CONFIG_32
                if (context.prec() == MPD_MAX_PREC) ulimit = 16000000;
            #endif
            Dec_DecCtx(token, &Decimal::sqrt);
            #ifdef CONFIG_32
                if (context.prec() == MPD_MAX_PREC) ulimit = SIZE_MAX;
            #endif
        }
        else if (eqtoken(token.at(1), "quantize_squareroot")) {
            #ifdef CONFIG_32
                if (context.prec() == MPD_MAX_PREC) ulimit = 16000000;
            #endif
            Dec_DecCtxWithQuantize(token, &Decimal::sqrt);
            #ifdef CONFIG_32
                if (context.prec() == MPD_MAX_PREC) ulimit = SIZE_MAX;
            #endif
        }
        else if (eqtoken(token.at(1), "tointegral")) {
            Dec_DecCtx(token, &Decimal::to_integral);
        }
        else if (eqtoken(token.at(1), "tointegralx")) {
            Dec_DecCtx(token, &Decimal::to_integral_exact);
        }
        else if (eqtoken(token.at(1), "floor")) {
            Dec_DecCtx(token, &Decimal::floor);
        }
        else if (eqtoken(token.at(1), "ceil")) {
            Dec_DecCtx(token, &Decimal::ceil);
        }
        else if (eqtoken(token.at(1), "trunc")) {
            Dec_DecCtx(token, &Decimal::trunc);
        }

        /* Binary function returning an int */
        else if (eqtoken(token.at(1), "samequantum")) {
            Bool_DecDec(SKIP_NONE, token, &Decimal::same_quantum);
        }

        /* Binary function returning an int, equal operands */
        else if (eqtoken(token.at(1), "samequantum_eq")) {
            Bool_DecDec(SKIP_NONE, token, &Decimal::same_quantum, true);
        }

        /* Binary functions with Decimal result */
        else if (eqtoken(token.at(1), "add")) {
            Dec_DecDecCtx(token, &Decimal::add);
            Dec_DecDec(token, &Decimal::operator+);
        }
        else if (eqtoken(token.at(1), "and")) {
            Dec_DecDecCtx(token, &Decimal::logical_and);
        }
        else if (eqtoken(token.at(1), "copysign")) {
            Dec_DecDec(token, &Decimal::copy_sign);
        }
        else if (eqtoken(token.at(1), "divide")) {
            #ifdef CONFIG_32
                if (context.prec() == MPD_MAX_PREC) ulimit = 16000000;
            #endif
            Dec_DecDecCtx(token, &Decimal::div, false, generated);
            Dec_DecDec(token, &Decimal::operator/, false, generated);
            #ifdef CONFIG_32
                if (context.prec() == MPD_MAX_PREC) ulimit = SIZE_MAX;
            #endif
        }
        else if (eqtoken(token.at(1), "divideint")) {
            Dec_DecDecCtx(token, &Decimal::divint, false, generated);
        }
        else if (eqtoken(token.at(1), "max")) {
            Dec_DecDecCtx(token, &Decimal::max);
        }
        else if (eqtoken(token.at(1), "maxmag") || eqtoken(token.at(1), "max_mag")) {
            Dec_DecDecCtx(token,  &Decimal::max_mag);
        }
        else if (eqtoken(token.at(1), "min")) {
            Dec_DecDecCtx(token, &Decimal::min);
        }
        else if (eqtoken(token.at(1), "minmag") || eqtoken(token.at(1), "min_mag")) {
            Dec_DecDecCtx(token, &Decimal::min_mag);
        }
        else if (eqtoken(token.at(1), "multiply")) {
            Dec_DecDecCtx(token, &Decimal::mul);
            Dec_DecDec(token, &Decimal::operator*);
        }
        else if (eqtoken(token.at(1), "nexttoward")) {
            Dec_DecDecCtx(token, &Decimal::next_toward);
        }
        else if (eqtoken(token.at(1), "or")) {
            Dec_DecDecCtx(token, &Decimal::logical_or);
        }
        else if (eqtoken(token.at(1), "power")) {
            if (!generated) {
                context.allcr(0);
                Dec_DecDecCtx(token, &Decimal::pow);
                context.allcr(1);
            }
            Dec_DecDecCtx(token, &Decimal::pow);
        }
        else if (eqtoken(token.at(1), "quantize")) {
            Dec_DecDecCtx(token, &Decimal::quantize);
        }
        else if (eqtoken(token.at(1), "resc")) {
            Dec_DecInt64Ctx(token, &Decimal::rescale);
        }
        else if (eqtoken(token.at(1), "remainder")) {
            Dec_DecDecCtx(token, &Decimal::rem, false, generated);
            Dec_DecDec(token, &Decimal::operator%, false, generated);
        }
        else if (eqtoken(token.at(1), "remaindernear")) {
            Dec_DecDecCtx(token, &Decimal::rem_near, false, generated);
        }
        else if (eqtoken(token.at(1), "rotate")) {
            Dec_DecDecCtx(token, &Decimal::rotate);
        }
        else if (eqtoken(token.at(1), "scaleb")) {
            Dec_DecDecCtx(token, &Decimal::scaleb);
        }
        else if (eqtoken(token.at(1), "shift")) {
            Dec_DecDecCtx(token, &Decimal::shift);
            if (!generated) {
                Dec_DecInt64Ctx(token, &Decimal::shiftn);
            }
        }
        else if (eqtoken(token.at(1), "subtract")) {
            Dec_DecDecCtx(token, &Decimal::sub);
            Dec_DecDec(token, &Decimal::operator-);
        }
        else if (eqtoken(token.at(1), "xor")) {
            Dec_DecDecCtx(token, &Decimal::logical_xor);
        }

        /* Binary functions with Decimal result, equal operands */
        else if (eqtoken(token.at(1), "add_eq")) {
            Dec_DecDecCtx(token, &Decimal::add, true);
            Dec_DecDec(token, &Decimal::operator+, true);
        }
        else if (eqtoken(token.at(1), "and_eq")) {
            Dec_DecDecCtx(token, &Decimal::logical_and, true);
        }
        else if (eqtoken(token.at(1), "copysign_eq")) {
            Dec_DecDec(token, &Decimal::copy_sign, true);
        }
        else if (eqtoken(token.at(1), "divide_eq")) {
            Dec_DecDecCtx(token, &Decimal::div, true);
            Dec_DecDec(token, &Decimal::operator/, true);
        }
        else if (eqtoken(token.at(1), "divideint_eq")) {
            Dec_DecDecCtx(token, &Decimal::divint, true);
        }
        else if (eqtoken(token.at(1), "max_eq")) {
            Dec_DecDecCtx(token, &Decimal::max, true);
        }
        else if (eqtoken(token.at(1), "maxmag_eq")) {
            Dec_DecDecCtx(token, &Decimal::max_mag, true);
        }
        else if (eqtoken(token.at(1), "min_eq")) {
            Dec_DecDecCtx(token, &Decimal::min, true);
        }
        else if (eqtoken(token.at(1), "minmag_eq")) {
            Dec_DecDecCtx(token, &Decimal::min_mag, true);
        }
        else if (eqtoken(token.at(1), "multiply_eq")) {
            Dec_DecDecCtx(token, &Decimal::mul, true);
            Dec_DecDec(token, &Decimal::operator*, true);
        }
        else if (eqtoken(token.at(1), "nexttoward_eq")) {
            Dec_DecDecCtx(token, &Decimal::next_toward, true);
        }
        else if (eqtoken(token.at(1), "or_eq")) {
            Dec_DecDecCtx(token, &Decimal::logical_or, true);
        }
        else if (eqtoken(token.at(1), "power_eq")) {
            if (!generated) {
                context.allcr(0);
                Dec_DecDecCtx(token, &Decimal::pow, true);
                context.allcr(1);
            }
            Dec_DecDecCtx(token, &Decimal::pow, true);
        }
        else if (eqtoken(token.at(1), "quantize_eq")) {
            Dec_DecDecCtx(token, &Decimal::quantize, true);
        }
        else if (eqtoken(token.at(1), "remainder_eq")) {
            Dec_DecDecCtx(token, &Decimal::rem, true);
            Dec_DecDec(token, &Decimal::operator%, true);
        }
        else if (eqtoken(token.at(1), "remaindernear_eq")) {
            Dec_DecDecCtx(token, &Decimal::rem_near, true);
        }
        else if (eqtoken(token.at(1), "rotate_eq")) {
            Dec_DecDecCtx(token, &Decimal::rotate, true);
        }
        else if (eqtoken(token.at(1), "scaleb_eq")) {
            Dec_DecDecCtx(token, &Decimal::scaleb, true);
        }
        else if (eqtoken(token.at(1), "shift_eq")) {
            Dec_DecDecCtx(token, &Decimal::shift, true);
        }
        else if (eqtoken(token.at(1), "subtract_eq")) {
            Dec_DecDecCtx(token, &Decimal::sub, true);
            Dec_DecDec(token, &Decimal::operator-, true);
        }
        else if (eqtoken(token.at(1), "xor_eq")) {
            Dec_DecDecCtx(token, &Decimal::logical_xor, true);
        }

        /* Binary function with Decimal pair result */
        else if (eqtoken(token.at(1), "divmod")) {
            DecPair_DecDecCtx(token, &Decimal::divmod);
        }
        /* Binary function with Decimal pair result, equal operands */
        else if (eqtoken(token.at(1), "divmod_eq")) {
            DecPair_DecDecCtx(token, &Decimal::divmod, true);
        }

        /* Ternary functions with Decimal result */
        else if (eqtoken(token.at(1), "fma")) {
            Dec_DecDecDecCtx(token, &Decimal::fma);
        }
        else if (eqtoken(token.at(1), "powmod")) {
            Dec_DecDecDecCtx(token, &Decimal::powmod);
        }

        /* Ternary functions with Decimal result, eq_eq_op */
        else if (eqtoken(token.at(1), "fma_eq_eq_op")) {
            Dec_DecDecDecCtx(token, &Decimal::fma, EqEqOp);
        }
        else if (eqtoken(token.at(1), "powmod_eq_eq_op")) {
            Dec_DecDecDecCtx(token, &Decimal::powmod, EqEqOp);
        }

        /* Ternary functions with Decimal result, eq_op_eq */
        else if (eqtoken(token.at(1), "fma_eq_op_eq")) {
            Dec_DecDecDecCtx(token, &Decimal::fma, EqOpEq);
        }
        else if (eqtoken(token.at(1), "powmod_eq_op_eq")) {
            Dec_DecDecDecCtx(token, &Decimal::powmod, EqOpEq);
        }

        /* Ternary functions with Decimal result, op_eq_eq */
        else if (eqtoken(token.at(1), "fma_op_eq_eq")) {
            Dec_DecDecDecCtx(token, &Decimal::fma, OpEqEq);
        }
        else if (eqtoken(token.at(1), "powmod_op_eq_eq")) {
            Dec_DecDecDecCtx(token, &Decimal::powmod, OpEqEq);
        }

        /* Ternary functions with Decimal result, eq_eq_eq */
        else if (eqtoken(token.at(1), "fma_eq_eq_eq")) {
            Dec_DecDecDecCtx(token, &Decimal::fma, EqEqEq);
        }
        else if (eqtoken(token.at(1), "powmod_eq_eq_eq")) {
            Dec_DecDecDecCtx(token, &Decimal::powmod, EqEqEq);
        }

        /* Special cases for the comparison functions */
        else if (eqtoken(token.at(1), "compare")) {
            Dec_DecDecCtx(token, &Decimal::compare);
            Int_DecDec(SKIP_NAN, token, &Decimal::cmp);
        }
        else if (eqtoken(token.at(1), "comparesig")) {
            Dec_DecDecCtx(token, &Decimal::compare_signal);
        }

        else if (eqtoken(token.at(1), "comparetotal")) {
            Dec_DecDec(token, &Decimal::compare_total);
            Int_DecDec(SKIP_NONE, token, &Decimal::cmp_total);
        }
        else if (eqtoken(token.at(1), "comparetotmag")) {
            Dec_DecDec(token, &Decimal::compare_total_mag);
            Int_DecDec(SKIP_NONE, token, &Decimal::cmp_total_mag);
        }

        /* Special cases for the comparison functions, equal operands */
        else if (eqtoken(token.at(1), "compare_eq")) {
            Dec_DecDecCtx(token, &Decimal::compare, true);
            Int_DecDec(SKIP_NAN, token, &Decimal::cmp, true);
        }
        else if (eqtoken(token.at(1), "comparesig_eq")) {
            Dec_DecDecCtx(token, &Decimal::compare_signal, true);
        }

        else if (eqtoken(token.at(1), "comparetotal_eq")) {
            Dec_DecDec(token, &Decimal::compare_total, true);
            Int_DecDec(SKIP_NAN, token, &Decimal::cmp_total, true);
        }
        else if (eqtoken(token.at(1), "comparetotmag_eq")) {
            Dec_DecDec(token, &Decimal::compare_total_mag, true);
            Int_DecDec(SKIP_NAN, token, &Decimal::cmp_total_mag, true);
        }

        /* Special cases for the shift functions */
        else if (eqtoken(token.at(1), "shiftleft")) {
            Dec_DecInt64Ctx(token, &Decimal::shiftl);
        }
        else if (eqtoken(token.at(1), "shiftright")) {
            Dec_DecInt64Ctx(token, &Decimal::shiftr);
        }

        /* Special case for Decimal::ln10() */
        else if (eqtoken(token.at(1), "ln10")) {
            ln10(token);
        }

        /* Special cases for the get_int functions */
        else if (eqtoken(token.at(1), "get_u64") || eqtoken(token.at(1), "get_uint64")) {
            u64_DecCtx(token);
        }
        else if (eqtoken(token.at(1), "get_u32") || eqtoken(token.at(1), "get_uint32")) {
            u32_DecCtx(token);
        }
        else if (eqtoken(token.at(1), "get_i64") || eqtoken(token.at(1), "get_int64")) {
            i64_DecCtx(token);
        }
        else if (eqtoken(token.at(1), "get_i32") || eqtoken(token.at(1), "get_int32")) {
            i32_DecCtx(token);
        }

        else if (startswith(token.at(0), "bool")) {
            continue;  // skip: bool tests in extra.decTest (not implemented in libmpdec++)
        }

        else if (eqtoken(token.at(1), "get_uint64_abs") ||
                 eqtoken(token.at(1), "get_ssize64") ||
                 eqtoken(token.at(1), "get_uint32_abs") ||
                 eqtoken(token.at(1), "get_ssize32")) {
            ;  // skip: not implemented in libmpdec++
        }

        else if (eqtoken(token.at(1), "rescale")) {
            ;  // skip: 'rescale' is obsolete in the standard and Decimal::rescale()
               // is not equivalent to the obsolete version.
        }
        else if (eqtoken(token.at(1), "baseconv")) {
            ;  // skip: not implemented in libmpdec++.
        }
        else {
            err_token(token, "unknown operation");
        }
    }
}

void
do_file(const std::string& filename, std::vector<std::string>& status, size_t i, bool threaded)
{
    try {
        if (threaded) {
            // Thread local context is initialized on first access.
            if (context.prec() != 1) {
                err_raise("automatic context initialization from template failed");
            }
        }

        std::ifstream in{filename};
        if (!in.is_open()) {
            err_raise("could not open ", filename);
        }

        do_stream(in);

        if (in.bad()) {
            err_raise("iterating over lines failed in ", filename);
        }
    } catch (Failure& e) {
        status[i] = e.what();
    }
}

/* process a file list */
int
do_files(const std::vector<std::string>& files)
{
    const size_t n = files.size();
    std::vector<std::string> status(n, "PASS");

    for (size_t i = 0; i < n; i++) {
        std::cerr << files[i] << " ... " << std::flush;
        do_file(files[i], status, i, false);
        std::cerr << status[i] << std::endl << std::flush;
    }

    return exit_status(status);
}

/* process a file list, threaded */
int
do_files_threaded(const std::vector<std::string>& files)
{
    const size_t n = files.size();
    std::vector<std::string> status(n, "PASS");
    std::vector<std::thread> t(n);

    for (size_t i = 0; i < n; i++) {
        t[i] = std::thread(do_file, files[i], std::ref(status), i, true);
    }

    for (size_t i = 0; i < n; i++) {
        t[i].join();
    }

    for (size_t i = 0; i < n; i++) {
        std::cerr << files[i] << " ... " << status[i] << std::endl << std::flush;
    }

    return exit_status(status);
}



#define err_exit(msg) \
  do {std::cerr << __FILE__ << ":" << __LINE__ << ": error: "; \
      std::cerr << msg  << std::endl;                          \
      std::exit(EXIT_FAILURE);                                 \
  } while (0)


namespace mpd {

#define MPD_CHECK(p, msg) if (!(p)) { err_exit(msg); }

const int32_t int32_cases[] = {
  INT32_MIN, INT32_MIN+1, INT32_MIN+2,
  INT32_MAX-2, INT32_MAX-1, INT32_MAX,
  -10, -5, -1, 0, 5, 10,
  -999999999, -99999999, -9999999, -999999, -99999, -9999, -999, -99, -9,
  -1000500001, -100050001, -10050001, -1005001, -105001, -10501, -1501, -151,
  -1000000001, -100000001, -10000001, -1000001, -100001, -10001, -1001, -101,
  -1000000000, -100000000, -10000000, -1000000, -100000, -10000, -1000, -100,
  999999999, 99999999, 9999999, 999999, 99999, 9999, 999, 99, 9,
  1000500001, 100050001, 10050001, 1005001, 105001, 10501, 1501, 151,
  1000000001, 100000001, 10000001, 1000001, 100001, 10001, 1001, 101,
  1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100,
  -(1<<30),
  -(1<<29), -(1<<28), -(1<<27), -(1<<26), -(1<<25), -(1<<24), -(1<<23), -(1<<22), -(1<<21), -(1<<20),
  -(1<<19), -(1<<18), -(1<<17), -(1<<16), -(1<<15), -(1<<14), -(1<<13), -(1<<12), -(1<<11), -(1<<10),
  -(1<<9),  -(1<<8),  -(1<<7),  -(1<<6),  -(1<<5),  -(1<<4),  -(1<<3),  -(1<<2),  -(1<<1),  -(1<<0),
   (1<<30),
   (1<<29), (1<<28), (1<<27), (1<<26), (1<<25), (1<<24), (1<<23), (1<<22), (1<<21), (1<<20),
   (1<<19), (1<<18), (1<<17), (1<<16), (1<<15), (1<<14), (1<<13), (1<<12), (1<<11), (1<<10),
   (1<<9),  (1<<8),  (1<<7),  (1<<6),  (1<<5),  (1<<4),  (1<<3),  (1<<2),  (1<<1),  (1<<0)
};

const int64_t int64_cases[] = {
  INT64_MIN, INT64_MIN+1, INT64_MIN+2, -10, -5, -1, 0, 5, 10, INT64_MAX-2, INT64_MAX-1, INT64_MAX,
  -999999999999999999LL, -99999999999999999LL, -9999999999999999LL, -999999999999999LL, -99999999999999LL, -9999999999999LL,
  -999999999999LL, -99999999999LL, -9999999999LL, -999999999LL, -99999999LL, -9999999LL, -999999LL, -99999LL, -9999LL, -999LL, -99LL, -9LL,
  -1000000000000000000LL, -100000000000000000LL, -10000000000000000LL, -1000000000000000LL, -100000000000000LL, -10000000000000LL,
  -1000000000000LL, -100000000000LL, -10000000000LL, -1000000000LL, -100000000LL, -10000000LL, -1000000LL, -100000LL, -10000LL, -1000LL, -100LL, -10LL,
  -1000000005000000000LL, -100000005000000000LL, -10000005000000000LL, -1000005000000000LL, -100000005000000LL, -10000005000000LL,
  -1000005000000LL, -100005000000LL, -10000005000LL, -1000005000LL, -100005000LL, -10005000LL, -1005000LL, -100050LL, -10050LL, -1050LL, -150LL, -15LL,
  -1000000005000000001LL, -100000005000000001LL, -10000005000000001LL, -1000005000000001LL, -100000005000001LL, -10000005000001LL,
  -1000005000001LL, -100005000001LL, -10000005001LL, -1000005001LL, -100005001LL, -10005001LL, -1005001LL, -100051LL, -10051LL, -1051LL, -151LL, -15LL,
  999999999999999999LL, 99999999999999999LL, 9999999999999999LL, 999999999999999LL, 99999999999999LL, 9999999999999LL,
  999999999999LL, 99999999999LL, 9999999999LL, 999999999LL, 99999999LL, 9999999LL, 999999LL, 99999LL, 9999LL, 999LL, 99LL, 9LL,
  1000000000000000000LL, 100000000000000000LL, 10000000000000000LL, 1000000000000000LL, 100000000000000LL, 10000000000000LL,
  1000000000000LL, 100000000000LL, 10000000000LL, 1000000000LL, 100000000LL, 10000000LL, 1000000LL, 100000LL, 10000LL, 1000LL, 100LL, 10LL,
  1000000005000000000LL, 100000005000000000LL, 10000005000000000LL, 1000005000000000LL, 100000005000000LL, 10000005000000LL,
  1000005000000LL, 100005000000LL, 10000005000LL, 1000005000LL, 100005000LL, 10005000LL, 1005000LL, 100050LL, 10050LL, 1050LL, 150LL, 15LL,
  1000000005000000001LL, 100000005000000001LL, 10000005000000001LL, 1000005000000001LL, 100000005000001LL, 10000005000001LL,
  1000005000001LL, 100005000001LL, 10000005001LL, 1000005001LL, 100005001LL, 10005001LL, 1005001LL, 100051LL, 10051LL, 1051LL, 151LL, 15LL,
  -(1LL<<62), -(1LL<<61), -(1LL<<60),
  -(1LL<<59), -(1LL<<58), -(1LL<<57), -(1LL<<56), -(1LL<<55), -(1LL<<54), -(1LL<<53), -(1LL<<52), -(1LL<<51), -(1LL<<50),
  -(1LL<<39), -(1LL<<38), -(1LL<<37), -(1LL<<36), -(1LL<<35), -(1LL<<34), -(1LL<<33), -(1LL<<32), -(1LL<<31), -(1LL<<30),
  -(1LL<<29), -(1LL<<28), -(1LL<<27), -(1LL<<26), -(1LL<<25), -(1LL<<24), -(1LL<<23), -(1LL<<22), -(1LL<<21), -(1LL<<20),
  -(1LL<<19), -(1LL<<18), -(1LL<<17), -(1LL<<16), -(1LL<<15), -(1LL<<14), -(1LL<<13), -(1LL<<12), -(1LL<<11), -(1LL<<10),
  -(1LL<<9),  -(1LL<<8),  -(1LL<<7),  -(1LL<<6),  -(1LL<<5),  -(1LL<<4),  -(1LL<<3),  -(1LL<<2),  -(1LL<<1),  -(1LL<<0),
  -(1LL<<62), -(1LL<<61), -(1LL<<60),
  (1LL<<59), (1LL<<58), (1LL<<57), (1LL<<56), (1LL<<55), (1LL<<54), (1LL<<53), (1LL<<52), (1LL<<51), (1LL<<50),
  (1LL<<39), (1LL<<38), (1LL<<37), (1LL<<36), (1LL<<35), (1LL<<34), (1LL<<33), (1LL<<32), (1LL<<31), (1LL<<30),
  (1LL<<29), (1LL<<28), (1LL<<27), (1LL<<26), (1LL<<25), (1LL<<24), (1LL<<23), (1LL<<22), (1LL<<21), (1LL<<20),
  (1LL<<19), (1LL<<18), (1LL<<17), (1LL<<16), (1LL<<15), (1LL<<14), (1LL<<13), (1LL<<12), (1LL<<11), (1LL<<10),
  (1LL<<9),  (1LL<<8),  (1LL<<7),  (1LL<<6),  (1LL<<5),  (1LL<<4),  (1LL<<3),  (1LL<<2),  (1LL<<1),  (1LL<<0),
};

const char *init_cases[] = {
  "sNaN", "sNaN19",
  "sNaN1982612612300000002000000000050000000000000000101111111111111112111111111111111111111111111111111111111111111111"
  "111111111111111111111111111111111111111111111111111111111111111",
  "-sNaN", "-sNaN19",
  "-sNaN198261261230000000200000000005000000000000000010111111111111111211111111111111111111111111111111111111111111111"
  "1111111111111111111111111111111111111111111111111111111111111111",
  "NaN", "NaN19",
  "NaN19826126123000000020000000000500000000000000001011111111111111121111111111111111111111111111111111111111111111111"
  "11111111111111111111111111111111111111111111111111111111111111",
  "-NaN", "-NaN19",
  "-NaN1982612612300000002000000000050000000000000000101111111111111112111111111111111111111111111111111111111111111111"
  "111111111111111111111111111111111111111111111111111111111111111",
  "inf", "-inf",
  "-1", "-0", "0", "1",
  "1e10", "-1e10",
  "1.21019218731291112376416152e10",
  "-1.21019218731291112376416152e10",
  "0.0000000000000000000000000000000000000000000000000001e-999999",
  "-0.0000000000000000000000000000000000000000000000000001e-999999"
};

void
test_set_i32(void)
{
    const Context savecontext = context;
    context.status(0);
    for (const char *s : init_cases) {
        for (const int32_t& x : int32_cases) {
            Decimal v{s};

            v = x;
            assertEqual(context.status(), 0U);
            assertEqualStr(v, std::to_string(x));
        }
    }
    context = savecontext;
}

void
test_set_i64(void)
{
    const Context savecontext = context;
    context.status(0);
    for (const char *s : init_cases) {
        for (const int64_t& x : int64_cases) {
            Decimal v{s};

            v = x;
            assertEqual(context.status(), 0U);
            assertEqualStr(v, std::to_string(x));
        }
    }
    context = savecontext;
}

}  /* namespace mpd */
}  /* namespace test */


namespace run {
/* process a single test file */
void
usage(void)
{
    std::cerr << "runtest: usage: runtest testfile [--threaded]" << std::endl;
    exit(EXIT_FAILURE);
}

std::vector<std::string>
collect_files(const std::string& topfile)
{
    std::vector<std::string> files;
    std::string line;

    std::ifstream in{topfile};
    if (!in.is_open()) {
        err_exit("could not open file");
    }

    while (std::getline(in, line)) {
        std::vector<std::string> token = test::split(line);
        if (token.size() == 0) {
            continue;
        }

        if (test::startswith(token.at(0), "Dectest")) {
            files.push_back(token.at(1));
            continue;
        }
        else {
            err_exit("parse error");
        }
    }

    if (in.bad()) {
        err_exit("iterating over lines failed");
    }

    return files;
}
}  /* namespace run */

int
main(int argc, char **argv)
{
    bool threaded = false;
    std::string filename;

    if (argc == 2) {
        filename = std::string(argv[1]);
    }
    else if (argc == 3) {
        filename = std::string(argv[1]);
        if (std::string(argv[2]) != "--threaded") {
            run::usage();
        }
        threaded = true;
    }
    else {
        run::usage();
    }

    static_assert(MPD_MAJOR_VERSION == 2, "MPD_MAJOR_VERSION must be 2");
    static_assert(MPD_MINOR_VERSION == 5, "MPD_MINOR_VERSION must be 5");
    static_assert(MPD_MICRO_VERSION == 0, "MPD_MICRO_VERSION must be 0");

    if (strcmp(mpd_version(), "2.5.0") != 0) {
        err_exit("mpd_version() != 2.5.0");
    }
    if (strcmp(MPD_VERSION, "2.5.0") != 0) {
        err_exit("MPD_VERSION != 2.5.0");
    }

    srand((unsigned int)time(NULL));

    // Initialize custom alloction functions.
    test::init_alloc_fail();

    // Initialize the context initialization template.
    context_template = Context(1, 1, -1);
    // With gcc and clang, the thread local context is initialized on first access
    // also for the main thread.  This is implementation-defined, the test is here
    // to remind people that the behavior is not guaranteed.
#if defined(__GNUC__) && !defined(__INTEL_COMPILER)
    if (context.prec() != 1) {
        err_exit("main thread: automatic context initialization from template failed");
    }
#endif

    test::mpd::test_set_i32();
    test::mpd::test_set_i64();

    if (filename == "-") {
        try {
            test::do_stream(std::cin, true);
        }
        catch (test::Failure& e) {
            std::cerr << "<stdin> ... " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        std::cerr << "<stdin> ... PASS" << std::endl << std::flush;
        return EXIT_SUCCESS;
    }

    std::vector<std::string> files;

    if (test::endswith(filename, ".decTest")) {
        files.push_back(filename);
    }
    else if (test::endswith(filename, ".topTest")) {
        std::ifstream in{filename};
        if (!in.is_open()) {
            err_exit("could not open file");
        }

        files = run::collect_files(filename);

        if (in.bad()) {
            err_exit("iterating over lines failed");
        }
    }
    else {
        err_exit("unrecognized file extension");
    }

    if (threaded) {
        return test::do_files_threaded(files);
    }
    else {
        return test::do_files(files);
    }
}
