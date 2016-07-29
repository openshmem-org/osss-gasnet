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
extern void shmemx_complexf_put_nbi (COMPLEXIFY (float) * dest,
                                const COMPLEXIFY (float) * src,
                                size_t nelems, int pe);  /* ! API */
extern void shmemx_complexd_put_nbi (COMPLEXIFY (double) * dest,
                                const COMPLEXIFY (double) * src,
                                size_t nelems, int pe);    /* ! API */
#pragma weak shmemx_short_put_nbi = pshmemx_short_put
#define shmemx_short_put_nbi pshmemx_short_put
#pragma weak shmemx_int_put_nbi = pshmemx_int_put
#define shmemx_int_put_nbi pshmemx_int_put
#pragma weak shmemx_long_put_nbi = pshmemx_long_put
#define shmemx_long_put_nbi pshmemx_long_put
#pragma weak shmemx_longdouble_put_nbi = pshmemx_longdouble_put
#define shmemx_longdouble_put_nbi pshmemx_longdouble_put
#pragma weak shmemx_longlong_put_nbi = pshmemx_longlong_put
#define shmemx_longlong_put_nbi pshmemx_longlong_put
#pragma weak shmemx_double_put_nbi = pshmemx_double_put
#define shmemx_double_put_nbi pshmemx_double_put
#pragma weak shmemx_float_put_nbi = pshmemx_float_put
#define shmemx_float_put_nbi pshmemx_float_put
#pragma weak shmemx_complexf_put_nbi = pshmemx_complexf_put
#define shmemx_complexf_put_nbi pshmemx_complexf_put
#pragma weak shmemx_complexd_put_nbi = pshmemx_complexd_put
#define shmemx_complexd_put_nbi pshmemx_complexd_put
#pragma weak shmemx_putmem_nbi = pshmemx_putmem
#define shmemx_putmem_nbi pshmemx_putmem
#pragma weak shmemx_put32_nbi = pshmemx_put32_nbi
#define shmemx_put32_nbi pshmemx_put32_nbi
#pragma weak shmemx_put64_nbi = pshmemx_put64_nbi
#define shmemx_put64_nbi pshmemx_put64_nbi
#pragma weak shmemx_put128_nbi = pshmemx_put128_nbi
#define shmemx_put128_nbi pshmemx_put128_nbi
/* # pragma weak pshmemx_put_nbi = pshmemx_long_put_nbi */
/* # pragma weak shmemx_put_nbi = pshmemx_put_nbi */
#endif /* HAVE_FEATURE_PSHMEM */

/*
 * do typed puts.  NB any short-circuits or address translations are
 * now deferred to the comms layer
 */

#define SHMEM_TYPE_PUT_NBI(Name, Type)                                  \
    void                                                                \
    shmemx_##Name##_put_nbi (Type *dest, const Type *src,               \
                             size_t nelems, int pe)                     \
    {                                                                   \
        const size_t typed_nelems = nelems * sizeof (Type);             \
        INIT_CHECK ();                                                  \
        SYMMETRY_CHECK (dest, 1, "shmemx_" #Name "_put_nbi");           \
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
shmemx_put32_nbi (void *dest, const void *src, size_t nelems, int pe)
{
    shmemx_int_put_nbi (dest, src, nelems, pe);
}

void
shmemx_put64_nbi (void *dest, const void *src, size_t nelems, int pe)
{
    shmemx_long_put_nbi (dest, src, nelems, pe);
}

void
shmemx_put128_nbi (void *dest, const void *src, size_t nelems, int pe)
{
    shmemx_longdouble_put_nbi (dest, src, nelems, pe);
}

void
shmemx_putmem_nbi (void *dest, const void *src, size_t nelems, int pe)
{
    INIT_CHECK ();
    SYMMETRY_CHECK (dest, 1, "shmemx_putmem_nbi");
    PE_RANGE_CHECK (pe, 4);
    shmemi_comms_put_nbi_bulk (dest, (void *) src, nelems, pe);
}


#ifdef HAVE_FEATURE_PSHMEM
extern void shmemx_complexf_get_nbi (COMPLEXIFY (float) * dest,
                                const COMPLEXIFY (float) * src,
                                size_t nelems, int pe);  /* ! API */
extern void shmemx_complexd_get_nbi (COMPLEXIFY (double) * dest,
                                const COMPLEXIFY (double) * src,
                                size_t nelems, int pe);    /* ! API */
#pragma weak shmemx_short_get_nbi = pshmemx_short_get
#define shmemx_short_get_nbi pshmemx_short_get
#pragma weak shmemx_int_get_nbi = pshmemx_int_get
#define shmemx_int_get_nbi pshmemx_int_get
#pragma weak shmemx_long_get_nbi = pshmemx_long_get
#define shmemx_long_get_nbi pshmemx_long_get
#pragma weak shmemx_longdouble_get_nbi = pshmemx_longdouble_get
#define shmemx_longdouble_get_nbi pshmemx_longdouble_get
#pragma weak shmemx_longlong_get_nbi = pshmemx_longlong_get
#define shmemx_longlong_get_nbi pshmemx_longlong_get
#pragma weak shmemx_double_get_nbi = pshmemx_double_get
#define shmemx_double_get_nbi pshmemx_double_get
#pragma weak shmemx_float_get_nbi = pshmemx_float_get
#define shmemx_float_get_nbi pshmemx_float_get
#pragma weak shmemx_complexf_get_nbi = pshmemx_complexf_get
#define shmemx_complexf_get_nbi pshmemx_complexf_get
#pragma weak shmemx_complexd_get_nbi = pshmemx_complexd_get
#define shmemx_complexd_get_nbi pshmemx_complexd_get
#pragma weak shmemx_getmem_nbi = pshmemx_getmem
#define shmemx_getmem_nbi pshmemx_getmem
#pragma weak shmemx_get32_nbi = pshmemx_get32_nbi
#define shmemx_get32_nbi pshmemx_get32_nbi
#pragma weak shmemx_get64_nbi = pshmemx_get64_nbi
#define shmemx_get64_nbi pshmemx_get64_nbi
#pragma weak shmemx_get128_nbi = pshmemx_get128_nbi
#define shmemx_get128_nbi pshmemx_get128_nbi
/* # pragma weak pshmemx_get_nbi = pshmemx_long_get_nbi */
/* # pragma weak shmemx_get_nbi = pshmemx_get_nbi */
#endif /* HAVE_FEATURE_PSHMEM */

#define SHMEM_TYPE_GET_NBI(Name, Type)                                  \
    void                                                                \
    shmemx_##Name##_get_nbi (Type *dest, const Type *src,               \
                             size_t nelems, int pe)                     \
    {                                                                   \
        const size_t typed_nelems = nelems * sizeof (Type);             \
        INIT_CHECK ();                                                  \
        SYMMETRY_CHECK (src, 2, "shmemx_" #Name "_get_nbi");            \
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
shmemx_get32_nbi (void *dest, const void *src, size_t nelems, int pe)
{
    shmemx_int_get_nbi (dest, src, nelems, pe);
}

void
shmemx_get64_nbi (void *dest, const void *src, size_t nelems, int pe)
{
    shmemx_long_get_nbi (dest, src, nelems, pe);
}

void
shmemx_get128_nbi (void *dest, const void *src, size_t nelems, int pe)
{
    shmemx_longdouble_get_nbi (dest, src, nelems, pe);
}

void
shmemx_getmem_nbi (void *dest, const void *src, size_t nelems, int pe)
{
    INIT_CHECK ();
    SYMMETRY_CHECK (src, 2, "shmemx_getmem_nbi");
    PE_RANGE_CHECK (pe, 4);
    shmemi_comms_get_nbi_bulk (dest, (void *) src, nelems, pe);
}
