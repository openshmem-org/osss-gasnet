/*
 *
 * Copyright (c) 2011 - 2015
 *   University of Houston System and UT-Battelle, LLC.
 * Copyright (c) 2009 - 2015
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



#if defined(HAVE_FEATURE_EXPERIMENTAL)

#include "state.h"
#include "trace.h"
#include "utils.h"

#include "shmem.h"

#include "comms/comms.h"

#ifdef HAVE_FEATURE_PSHMEM
#include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */


#ifdef HAVE_FEATURE_PSHMEM
extern void shmemx_put16_nb (void *dest, const void *src,
                             size_t nelems, int pe,
                             shmemx_request_handle_t * desc);   /* ! API */
extern void shmemx_put_nb (long *dest, const long *src,
                           size_t nelems, int pe,
                           shmemx_request_handle_t * desc); /* ! API */
#pragma weak shmemx_short_put_nb = pshmemx_short_put_nb
#define shmemx_short_put_nb pshmemx_short_put_nb
#pragma weak shmemx_int_put_nb = pshmemx_int_put_nb
#define shmemx_int_put_nb pshmemx_int_put_nb
#pragma weak shmemx_long_put_nb = pshmemx_long_put_nb
#define shmemx_long_put_nb pshmemx_long_put_nb
#pragma weak shmemx_longdouble_put_nb = pshmemx_longdouble_put_nb
#define shmemx_longdouble_put_nb pshmemx_longdouble_put_nb
#pragma weak shmemx_longlong_put_nb = pshmemx_longlong_put_nb
#define shmemx_longlong_put_nb pshmemx_longlong_put_nb
#pragma weak shmemx_double_put_nb = pshmemx_double_put_nb
#define shmemx_double_put_nb pshmemx_double_put_nb
#pragma weak shmemx_float_put_nb = pshmemx_float_put_nb
#define shmemx_float_put_nb pshmemx_float_put_nb
#pragma weak shmemx_put16_nb = pshmemx_put16_nb
#define shmemx_put16_nb pshmemx_put16_nb
#pragma weak shmemx_put32_nb = pshmemx_put32_nb
#define shmemx_put32_nb pshmemx_put32_nb
#pragma weak shmemx_put64_nb = pshmemx_put64_nb
#define shmemx_put64_nb pshmemx_put64_nb
#pragma weak shmemx_put128_nb = pshmemx_put128_nb
#define shmemx_put128_nb pshmemx_put128_nb
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * non-blocking extensions
 */

#define SHMEMX_TYPE_PUT_NB(Name, Type)                                  \
    void                                                                \
    shmemx_##Name##_put_nb (Type *target, const Type *source, size_t nelems, \
                            int pe, shmemx_request_handle_t *desc)      \
    {                                                                   \
        int typed_nelems = sizeof(Type) * nelems;                       \
        INIT_CHECK ();                                                  \
        SYMMETRY_CHECK (target, 1, "shmemx_" #Name "_put_nb");          \
        PE_RANGE_CHECK (pe, 4);                                         \
        shmemi_comms_put_nb ((Type *) target, (Type *) source,          \
                             typed_nelems, pe, desc);                   \
    }

SHMEMX_TYPE_PUT_NB (short, short);
SHMEMX_TYPE_PUT_NB (int, int);
SHMEMX_TYPE_PUT_NB (long, long);
SHMEMX_TYPE_PUT_NB (longdouble, long double);
SHMEMX_TYPE_PUT_NB (longlong, long long);
SHMEMX_TYPE_PUT_NB (double, double);
SHMEMX_TYPE_PUT_NB (float, float);

void
shmemx_put16_nb (void *dest, const void *src, size_t nelems,
                 int pe, shmemx_request_handle_t * desc)
{
    shmemx_short_put_nb (dest, src, nelems, pe, desc);
}

void
shmemx_put32_nb (void *dest, const void *src, size_t nelems,
                 int pe, shmemx_request_handle_t * desc)
{
    shmemx_int_put_nb (dest, src, nelems, pe, desc);
}

void
shmemx_put64_nb (void *dest, const void *src, size_t nelems,
                 int pe, shmemx_request_handle_t * desc)
{
    shmemx_long_put_nb (dest, src, nelems, pe, desc);
}

void
shmemx_put128_nb (void *dest, const void *src, size_t nelems,
                  int pe, shmemx_request_handle_t * desc)
{
    shmemx_longdouble_put_nb (dest, src, nelems, pe, desc);
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmemx_putmem_nb = pshmemx_putmem_nb
#define shmemx_putmem_nb pshmemx_putmem_nb
#pragma weak shmemx_put_nb = pshmemx_put_nb
#define shmemx_put_nb pshmemx_put_nb
#endif /* HAVE_FEATURE_PSHMEM */

void
shmemx_putmem_nb (void *target, const void *source, size_t nelems,
                  int pe, shmemx_request_handle_t * desc)
{
    INIT_CHECK ();
    SYMMETRY_CHECK (target, 1, "shmemx_putmem_nb");
    PE_RANGE_CHECK (pe, 4);
    shmemi_comms_put_nb (target, (void *) source, nelems, pe, desc);
}

void
shmemx_put_nb (long *target, const long *source, size_t nelems,
               int pe, shmemx_request_handle_t * desc)
{
    shmemx_long_put_nb (target, source, nelems, pe, desc);
}

#ifdef HAVE_FEATURE_PSHMEM
extern void shmemx_get16_nb (void *dest, const void *src,
                             size_t nelems, int pe,
                             shmemx_request_handle_t * desc);   /* ! API */
extern void shmemx_get_nb (long *dest, const long *src,
                           size_t nelems, int pe,
                           shmemx_request_handle_t * desc); /* ! API */
#pragma weak shmemx_short_get_nb = pshmemx_short_get_nb
#define shmemx_short_get_nb pshmemx_short_get_nb
#pragma weak shmemx_int_get_nb = pshmemx_int_get_nb
#define shmemx_int_get_nb pshmemx_int_get_nb
#pragma weak shmemx_long_get_nb = pshmemx_long_get_nb
#define shmemx_long_get_nb pshmemx_long_get_nb
#pragma weak shmemx_longdouble_get_nb = pshmemx_longdouble_get_nb
#define shmemx_longdouble_get_nb pshmemx_longdouble_get_nb
#pragma weak shmemx_longlong_get_nb = pshmemx_longlong_get_nb
#define shmemx_longlong_get_nb pshmemx_longlong_get_nb
#pragma weak shmemx_double_get_nb = pshmemx_double_get_nb
#define shmemx_double_get_nb pshmemx_double_get_nb
#pragma weak shmemx_float_get_nb = pshmemx_float_get_nb
#define shmemx_float_get_nb pshmemx_float_get_nb
#pragma weak shmemx_get16_nb = pshmemx_get16_nb
#define shmemx_get16_nb pshmemx_get16_nb
#pragma weak shmemx_get32_nb = pshmemx_get32_nb
#define shmemx_get32_nb pshmemx_get32_nb
#pragma weak shmemx_get64_nb = pshmemx_get64_nb
#define shmemx_get64_nb pshmemx_get64_nb
#pragma weak shmemx_get128_nb = pshmemx_get128_nb
#define shmemx_get128_nb pshmemx_get128_nb
#endif /* HAVE_FEATURE_PSHMEM */

#define SHMEMX_TYPE_GET_NB(Name, Type)                                  \
    void                                                                \
    shmemx_##Name##_get_nb (Type *target, const Type *source, size_t nelems, \
                            int pe, shmemx_request_handle_t *desc)      \
    {                                                                   \
        int typed_nelems = sizeof(Type) * nelems;                       \
        INIT_CHECK ();                                                  \
        SYMMETRY_CHECK (source, 2, "shmemx_" #Name "_get_nb");          \
        PE_RANGE_CHECK (pe, 4);                                         \
        shmemi_comms_get_nb ((Type *) target, (Type *) source,          \
                             typed_nelems, pe, desc);                   \
    }

SHMEMX_TYPE_GET_NB (short, short);
SHMEMX_TYPE_GET_NB (int, int);
SHMEMX_TYPE_GET_NB (long, long);
SHMEMX_TYPE_GET_NB (longdouble, long double);
SHMEMX_TYPE_GET_NB (longlong, long long);
SHMEMX_TYPE_GET_NB (double, double);
SHMEMX_TYPE_GET_NB (float, float);

void
shmemx_get16_nb (void *dest, const void *src, size_t nelems,
                 int pe, shmemx_request_handle_t * desc)
{
    shmemx_short_get_nb (dest, src, nelems, pe, desc);
}

void
shmemx_get32_nb (void *dest, const void *src, size_t nelems,
                 int pe, shmemx_request_handle_t * desc)
{
    shmemx_int_get_nb (dest, src, nelems, pe, desc);
}

void
shmemx_get64_nb (void *dest, const void *src, size_t nelems,
                 int pe, shmemx_request_handle_t * desc)
{
    shmemx_long_get_nb (dest, src, nelems, pe, desc);
}

void
shmemx_get128_nb (void *dest, const void *src, size_t nelems,
                  int pe, shmemx_request_handle_t * desc)
{
    shmemx_longdouble_get_nb (dest, src, nelems, pe, desc);
}


#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmemx_getmem_nb = pshmemx_getmem_nb
#define shmemx_getmem_nb pshmemx_getmem_nb
#pragma weak shmemx_get_nb = pshmemx_get_nb
#define shmemx_get_nb pshmemx_get_nb
#endif /* HAVE_FEATURE_PSHMEM */

void
shmemx_getmem_nb (void *target, const void *source, size_t nelems,
                  int pe, shmemx_request_handle_t * desc)
{
    INIT_CHECK ();
    SYMMETRY_CHECK (source, 2, "shmemx_getmem_nb");
    PE_RANGE_CHECK (pe, 4);
    shmemi_comms_get_nb (target, (void *) source, nelems, pe, desc);
}

void
shmemx_get_nb (long *target, const long *source, size_t nelems,
               int pe, shmemx_request_handle_t * desc)
{
    shmemx_long_get_nb (target, source, nelems, pe, desc);
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmemx_wait_req = pshmemx_wait_req
#define shmemx_wait_req pshmemx_wait_req
#pragma weak shmemx_test_req = pshmemx_test_req
#define shmemx_test_req pshmemx_test_req
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * Wait for handle to be completed
 */

void
shmemx_wait_req (shmemx_request_handle_t desc)
{
    shmemi_comms_wait_req (desc);
}

/**
 * Test whether handle has been completed
 */

void
shmemx_test_req (shmemx_request_handle_t desc, int *flag)
{
    shmemi_comms_test_req (desc, flag);
}

#endif /* HAVE_FEATURE_EXPERIMENTAL */
