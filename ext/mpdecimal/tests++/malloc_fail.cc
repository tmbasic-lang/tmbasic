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


#include <cstdlib>
#include <cstdint>
#include <cinttypes>

#include "mpdecimal.h"

#include "decimal.hh"
#include "malloc_fail.hh"


/* Test allocation failures */
static thread_local uint64_t alloc_fail = UINT64_MAX;
static thread_local uint64_t alloc_idx = 0;

static void *
malloc_fail(size_t size)
{
#ifdef CONFIG_32
    if (size > test::ulimit) return NULL;
#endif

#ifdef TEST_ALLOC
    if (++alloc_idx >= alloc_fail-1) {
        return NULL;
    }
#endif

    return malloc(size);
}

static void *
calloc_fail(size_t nmemb, size_t size)
{
#ifdef CONFIG_32
    if (nmemb > test::ulimit / size) return NULL;
#endif

#ifdef TEST_ALLOC
    if (++alloc_idx >= alloc_fail-1) {
        return NULL;
    }
#endif

    return calloc(nmemb, size);
}

static void *
realloc_fail(void *ptr, size_t size)
{
#ifdef CONFIG_32
    if (size > test::ulimit) return NULL;
#endif

#ifdef TEST_ALLOC
    if (++alloc_idx >= alloc_fail-1) {
        return NULL;
    }
#endif

    return realloc(ptr, size);
}

namespace test {

/* Limit to simulate allocation failure for 32-bit MAX_PREC tests. */
#ifdef CONFIG_32
thread_local size_t ulimit = SIZE_MAX;
#endif

void
init_alloc_fail(void)
{
    mpd_mallocfunc = malloc_fail;
    mpd_callocfunc = calloc_fail;
    mpd_reallocfunc = realloc_fail;
}

void
set_alloc_fail(decimal::Context &ctx, uint64_t n)
{
    ctx.traps(MPD_Malloc_error);
    alloc_idx = 0;
    alloc_fail = n;
}

void
set_alloc(decimal::Context &ctx)
{
    ctx.traps(MPD_Malloc_error);
    alloc_idx = 0;
    alloc_fail = UINT64_MAX;
}

}  /* namespace test */
