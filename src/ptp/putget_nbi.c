/*
 *
 * Copyright (c) 2011 - 2016
 *   University of Houston System and UT-Battelle, LLC.
 * Copyright (c) 2009 - 2016
 *   Silicon Graphics International Corp.  SHMEM is copyrighted
 *   by Silicon Graphics International Corp. (SGI) The OpenSHMEM API
 *   (shmem) is released by Open Source Software Solutions, Inc., under an
 *   agreement with Silicon Graphics International Corp. (SGI).
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * o Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * o Neither the name of the University of Houston System,
 *   UT-Battelle, LLC. nor the names of its contributors may be used to
 *   endorse or promote products derived from this software without specific
 *   prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */



#include <stdio.h>              /* NULL */
#include <string.h>             /* memcpy() */
#include <sys/types.h>          /* size_t */

#include "state.h"
#include "globalvar.h"
#include "updown.h"
#include "trace.h"
#include "utils.h"
#include "symmtest.h"

#include "shmem.h"

#include "comms/comms.h"

#ifdef HAVE_FEATURE_PSHMEM
#include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */

#ifdef HAVE_FEATURE_PSHMEM
extern void shmem_complexf_put_nbi (COMPLEXIFY (float) * dest,
                                    const COMPLEXIFY (float) * src,
                                    size_t nelems, int pe);  /* ! API */
extern void shmem_complexd_put_nbi (COMPLEXIFY (double) * dest,
                                    const COMPLEXIFY (double) * src,
                                    size_t nelems, int pe);    /* ! API */
#pragma weak shmem_short_put_nbi = pshmem_short_put_nbi
#define shmem_short_put_nbi pshmem_short_put_nbi
#pragma weak shmem_int_put_nbi = pshmem_int_put_nbi
#define shmem_int_put_nbi pshmem_int_put_nbi
#pragma weak shmem_char_put_nbi = pshmem_char_put_nbi
#define shmem_char_put_nbi pshmem_char_put_nbi
#pragma weak shmem_long_put_nbi = pshmem_long_put_nbi
#define shmem_long_put_nbi pshmem_long_put_nbi
#pragma weak shmem_longdouble_put_nbi = pshmem_longdouble_put_nbi
#define shmem_longdouble_put_nbi pshmem_longdouble_put_nbi
#pragma weak shmem_longlong_put_nbi = pshmem_longlong_put_nbi
#define shmem_longlong_put_nbi pshmem_longlong_put_nbi
#pragma weak shmem_double_put_nbi = pshmem_double_put_nbi
#define shmem_double_put_nbi pshmem_double_put_nbi
#pragma weak shmem_float_put_nbi = pshmem_float_put_nbi
#define shmem_float_put_nbi pshmem_float_put_nbi
#pragma weak shmem_complexf_put_nbi = pshmem_complexf_put_nbi
#define shmem_complexf_put_nbi pshmem_complexf_put_nbi
#pragma weak shmem_complexd_put_nbi = pshmem_complexd_put_nbi
#define shmem_complexd_put_nbi pshmem_complexd_put_nbi
#pragma weak shmem_putmem_nbi = pshmem_putmem_nbi
#define shmem_putmem_nbi pshmem_putmem_nbi
#pragma weak shmem_put32_nbi = pshmem_put32_nbi
#define shmem_put32_nbi pshmem_put32_nbi
#pragma weak shmem_put64_nbi = pshmem_put64_nbi
#define shmem_put64_nbi pshmem_put64_nbi
#pragma weak shmem_put128_nbi = pshmem_put128_nbi
#define shmem_put128_nbi pshmem_put128_nbi
/* # pragma weak pshmem_put_nbi = pshmem_long_put_nbi */
/* # pragma weak shmem_put_nbi = pshmem_put_nbi */
#endif /* HAVE_FEATURE_PSHMEM */

/*
 * do typed puts.  NB any short-circuits or address translations are
 * now deferred to the comms layer
 */

#define SHMEM_TYPE_PUT_NBI(Name, Type)                                  \
    void                                                                \
    shmem_##Name##_put_nbi (Type *dest, const Type *src,                \
                            size_t nelems, int pe)                      \
    {                                                                   \
        const size_t typed_nelems = nelems * sizeof (Type);             \
        INIT_CHECK ();                                                  \
        SYMMETRY_CHECK (dest, 1, "shmem_" #Name "_put_nbi");            \
        PE_RANGE_CHECK (pe, 4);                                         \
        shmemi_comms_put_nbi (dest, (void *) src, typed_nelems, pe);    \
    }

SHMEM_TYPE_PUT_NBI (char, char);
SHMEM_TYPE_PUT_NBI (short, short);
SHMEM_TYPE_PUT_NBI (int, int);
SHMEM_TYPE_PUT_NBI (long, long);
SHMEM_TYPE_PUT_NBI (longlong, long long);
SHMEM_TYPE_PUT_NBI (longdouble, long double);
SHMEM_TYPE_PUT_NBI (double, double);
SHMEM_TYPE_PUT_NBI (float, float);
SHMEM_TYPE_PUT_NBI (complexf, COMPLEXIFY (float));
SHMEM_TYPE_PUT_NBI (complexd, COMPLEXIFY (double));

void
shmem_put32_nbi (void *dest, const void *src, size_t nelems, int pe)
{
    shmem_int_put_nbi (dest, src, nelems, pe);
}

void
shmem_put64_nbi (void *dest, const void *src, size_t nelems, int pe)
{
    shmem_long_put_nbi (dest, src, nelems, pe);
}

void
shmem_put128_nbi (void *dest, const void *src, size_t nelems, int pe)
{
    shmem_longdouble_put_nbi (dest, src, nelems, pe);
}

void
shmem_putmem_nbi (void *dest, const void *src, size_t nelems, int pe)
{
    INIT_CHECK ();
    SYMMETRY_CHECK (dest, 1, "shmem_putmem_nbi");
    PE_RANGE_CHECK (pe, 4);
    shmemi_comms_put_nbi_bulk (dest, (void *) src, nelems, pe);
}


#ifdef HAVE_FEATURE_PSHMEM
extern void shmem_complexf_get_nbi (COMPLEXIFY (float) * dest,
                                    const COMPLEXIFY (float) * src,
                                    size_t nelems, int pe);  /* ! API */
extern void shmem_complexd_get_nbi (COMPLEXIFY (double) * dest,
                                    const COMPLEXIFY (double) * src,
                                    size_t nelems, int pe);    /* ! API */
#pragma weak shmem_short_get_nbi = pshmem_short_get_nbi
#define shmem_short_get_nbi pshmem_short_get_nbi
#pragma weak shmem_int_get_nbi = pshmem_int_get_nbi
#define shmem_int_get_nbi pshmem_int_get_nbi
#pragma weak shmem_char_get_nbi = pshmem_char_get_nbi
#define shmem_char_get_nbi pshmem_char_get_nbi
#pragma weak shmem_long_get_nbi = pshmem_long_get_nbi
#define shmem_long_get_nbi pshmem_long_get_nbi
#pragma weak shmem_longdouble_get_nbi = pshmem_longdouble_get_nbi
#define shmem_longdouble_get_nbi pshmem_longdouble_get_nbi
#pragma weak shmem_longlong_get_nbi = pshmem_longlong_get_nbi
#define shmem_longlong_get_nbi pshmem_longlong_get_nbi
#pragma weak shmem_double_get_nbi = pshmem_double_get_nbi
#define shmem_double_get_nbi pshmem_double_get_nbi
#pragma weak shmem_float_get_nbi = pshmem_float_get_nbi
#define shmem_float_get_nbi pshmem_float_get_nbi
#pragma weak shmem_complexf_get_nbi = pshmem_complexf_get_nbi
#define shmem_complexf_get_nbi pshmem_complexf_get_nbi
#pragma weak shmem_complexd_get_nbi = pshmem_complexd_get_nbi
#define shmem_complexd_get_nbi pshmem_complexd_get_nbi
#pragma weak shmem_getmem_nbi = pshmem_getmem_nbi
#define shmem_getmem_nbi pshmem_getmem_nbi
#pragma weak shmem_get32_nbi = pshmem_get32_nbi
#define shmem_get32_nbi pshmem_get32_nbi
#pragma weak shmem_get64_nbi = pshmem_get64_nbi
#define shmem_get64_nbi pshmem_get64_nbi
#pragma weak shmem_get128_nbi = pshmem_get128_nbi
#define shmem_get128_nbi pshmem_get128_nbi
/* # pragma weak pshmem_get_nbi = pshmem_long_get_nbi */
/* # pragma weak shmem_get_nbi = pshmem_get_nbi */
#endif /* HAVE_FEATURE_PSHMEM */

#define SHMEM_TYPE_GET_NBI(Name, Type)                                  \
    void                                                                \
    shmem_##Name##_get_nbi (Type *dest, const Type *src,                \
                            size_t nelems, int pe)                      \
    {                                                                   \
        const size_t typed_nelems = nelems * sizeof (Type);             \
        INIT_CHECK ();                                                  \
        SYMMETRY_CHECK (src, 2, "shmem_" #Name "_get_nbi");             \
        PE_RANGE_CHECK (pe, 4);                                         \
        shmemi_comms_get_nbi (dest, (void *) src, typed_nelems, pe);    \
    }

SHMEM_TYPE_GET_NBI (char, char);
SHMEM_TYPE_GET_NBI (short, short);
SHMEM_TYPE_GET_NBI (int, int);
SHMEM_TYPE_GET_NBI (long, long);
SHMEM_TYPE_GET_NBI (longdouble, long double);
SHMEM_TYPE_GET_NBI (longlong, long long);
SHMEM_TYPE_GET_NBI (double, double);
SHMEM_TYPE_GET_NBI (float, float);
SHMEM_TYPE_GET_NBI (complexf, COMPLEXIFY (float));
SHMEM_TYPE_GET_NBI (complexd, COMPLEXIFY (double));;

void
shmem_get32_nbi (void *dest, const void *src, size_t nelems, int pe)
{
    shmem_int_get_nbi (dest, src, nelems, pe);
}

void
shmem_get64_nbi (void *dest, const void *src, size_t nelems, int pe)
{
    shmem_long_get_nbi (dest, src, nelems, pe);
}

void
shmem_get128_nbi (void *dest, const void *src, size_t nelems, int pe)
{
    shmem_longdouble_get_nbi (dest, src, nelems, pe);
}

void
shmem_getmem_nbi (void *dest, const void *src, size_t nelems, int pe)
{
    INIT_CHECK ();
    SYMMETRY_CHECK (src, 2, "shmem_getmem_nbi");
    PE_RANGE_CHECK (pe, 4);
    shmemi_comms_get_nbi_bulk (dest, (void *) src, nelems, pe);
}
