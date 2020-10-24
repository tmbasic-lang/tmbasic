/*
 * Copyright (c) 2008-2020 Stefan Krah. All rights reserved.
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


#include "mpdecimal.h"

#include <stdlib.h>

#include "malloc_fail.h"


/* Limit to simulate allocation failure for 32-bit MAX_PREC tests. */
#ifdef CONFIG_32
size_t ulimit = SIZE_MAX;
#endif

/* Test allocation failures */
int alloc_count;
int alloc_fail;
int alloc_idx;


void *
mpd_malloc_count(size_t size)
{
#ifdef CONFIG_32
     if (size > ulimit) return NULL;
#endif

    ++alloc_count;
    return malloc(size);
}

void *
mpd_calloc_count(size_t nmemb, size_t size)
{
#ifdef CONFIG_32
     if (nmemb > ulimit / size) return NULL;
#endif

    ++alloc_count;
    return calloc(nmemb, size);
}

void *
mpd_realloc_count(void *ptr, size_t size)
{
#ifdef CONFIG_32
     if (size > ulimit) return NULL;
#endif

    ++alloc_count;
    return realloc(ptr, size);
}


void *
mpd_malloc_fail(size_t size)
{
#ifdef CONFIG_32
     if (size > ulimit) return NULL;
#endif

#ifdef TEST_ALLOC
    if (++alloc_idx >= alloc_fail-1) {
        return NULL;
    }
#endif
    return malloc(size);
}

void *
mpd_calloc_fail(size_t nmemb, size_t size)
{
#ifdef CONFIG_32
     if (nmemb > ulimit / size) return NULL;
#endif

#ifdef TEST_ALLOC
    if (++alloc_idx >= alloc_fail-1) {
        return NULL;
    }
#endif
    if (rand()%2) {
        return calloc(nmemb, size);
    }
    else {
        return mpd_callocfunc_em(nmemb, size);
    }
}

void *
mpd_realloc_fail(void *ptr, size_t size)
{
#ifdef CONFIG_32
     if (size > ulimit) return NULL;
#endif

#ifdef TEST_ALLOC
    if (++alloc_idx >= alloc_fail-1) {
        return NULL;
    }
#endif
    return realloc(ptr, size);
}

void
mpd_set_alloc_count(mpd_context_t *ctx)
{
    mpd_mallocfunc = mpd_malloc_count;
    mpd_callocfunc = mpd_calloc_count;
    mpd_reallocfunc = mpd_realloc_count;

    ctx->traps = MPD_Malloc_error;
    alloc_count = 0;
}

void
mpd_set_alloc_fail(mpd_context_t *ctx)
{
    mpd_mallocfunc = mpd_malloc_fail;
    mpd_callocfunc = mpd_calloc_fail;
    mpd_reallocfunc = mpd_realloc_fail;

    ctx->traps = 0;
    alloc_idx = 0;
}

void
mpd_set_alloc(mpd_context_t *ctx)
{
    mpd_mallocfunc = malloc;
    mpd_callocfunc = (rand()%2) ? calloc : mpd_callocfunc_em;
    mpd_reallocfunc = realloc;

    ctx->traps = MPD_Malloc_error;
}
