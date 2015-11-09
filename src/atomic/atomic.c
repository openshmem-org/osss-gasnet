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



#include <sys/types.h>

#include "state.h"
#include "utils.h"
#include "atomic.h"

#include "shmem.h"

#include "comms/comms.h"


#ifdef HAVE_FEATURE_PSHMEM
#include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */


/*
 * placeholders: no init/final required (so far)
 */

void
shmemi_atomic_init (void)
{
}

void
shmemi_atomic_finalize (void)
{
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_int_swap = pshmem_int_swap
#define shmem_int_swap pshmem_int_swap
#pragma weak shmem_long_swap = pshmem_long_swap
#define shmem_long_swap pshmem_long_swap
#pragma weak shmem_longlong_swap = pshmem_longlong_swap
#define shmem_longlong_swap pshmem_longlong_swap
#pragma weak shmem_float_swap = pshmem_float_swap
#define shmem_float_swap pshmem_float_swap
#pragma weak shmem_double_swap = pshmem_double_swap
#define shmem_double_swap pshmem_double_swap
#pragma weak shmem_swap = pshmem_swap
#define shmem_swap pshmem_swap
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * shmem_swap performs an atomic swap operation. It writes value
 * "value" into target on processing element (PE) pe and returns the
 * previous contents of target as an atomic operation.
 */

#define SHMEM_TYPE_SWAP(Name, Type)                                     \
    Type                                                                \
    shmem_##Name##_swap (Type *target, Type value, int pe)              \
    {                                                                   \
        INIT_CHECK ();                                                  \
        PE_RANGE_CHECK (pe, 3);                                         \
        return shmemi_comms_swap_request_##Name (target, value,         \
                                                 pe);                   \
    }

SHMEM_TYPE_SWAP (int, int);
SHMEM_TYPE_SWAP (long, long);
SHMEM_TYPE_SWAP (longlong, long long);
SHMEM_TYPE_SWAP (double, double);
SHMEM_TYPE_SWAP (float, float);

/**
 * untyped variant
 */
long
shmem_swap (long *target, long value, int pe)
{
    return shmem_long_swap (target, value, pe);
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_int_cswap = pshmem_int_cswap
#define shmem_int_cswap pshmem_int_cswap
#pragma weak shmem_long_cswap = pshmem_long_cswap
#define shmem_long_cswap pshmem_long_cswap
#pragma weak shmem_longlong_cswap = pshmem_longlong_cswap
#define shmem_longlong_cswap pshmem_longlong_cswap
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * The conditional swap routines conditionally update a target data
 * object on an arbitrary processing element (PE) and return the prior
 * contents of the data object in one atomic operation.
 */

#define SHMEM_TYPE_CSWAP(Name, Type)                                    \
    Type                                                                \
    shmem_##Name##_cswap (Type *target, Type cond, Type value, int pe)  \
    {                                                                   \
        INIT_CHECK ();                                                  \
        PE_RANGE_CHECK (pe, 4);                                         \
        return shmemi_comms_cswap_request_##Name (target, cond, value,  \
                                                  pe);                  \
    }

SHMEM_TYPE_CSWAP (int, int);
SHMEM_TYPE_CSWAP (long, long);
SHMEM_TYPE_CSWAP (longlong, long long);

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_int_fadd = pshmem_int_fadd
#define shmem_int_fadd pshmem_int_fadd
#pragma weak shmem_long_fadd = pshmem_long_fadd
#define shmem_long_fadd pshmem_long_fadd
#pragma weak shmem_longlong_fadd = pshmem_longlong_fadd
#define shmem_longlong_fadd pshmem_longlong_fadd
#endif /* HAVE_FEATURE_PSHMEM */

#define SHMEM_TYPE_FADD(Name, Type)                                     \
    Type                                                                \
    shmem_##Name##_fadd (Type *target, Type value, int pe)              \
    {                                                                   \
        INIT_CHECK ();                                                  \
        PE_RANGE_CHECK (pe, 3);                                         \
        return shmemi_comms_fadd_request_##Name (target, value,         \
                                                 pe);                   \
    }

SHMEM_TYPE_FADD (int, int);
SHMEM_TYPE_FADD (long, long);
SHMEM_TYPE_FADD (longlong, long long);


#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_int_finc = pshmem_int_finc
#define shmem_int_finc pshmem_int_finc
#pragma weak shmem_long_finc = pshmem_long_finc
#define shmem_long_finc pshmem_long_finc
#pragma weak shmem_longlong_finc = pshmem_longlong_finc
#define shmem_longlong_finc pshmem_longlong_finc
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * finc performs an atomic fetch-and-increment at an address
 * on another PE
 */

#define SHMEM_TYPE_FINC(Name, Type)                             \
    Type                                                        \
    shmem_##Name##_finc (Type *target, int pe)                  \
    {                                                           \
        INIT_CHECK ();                                          \
        PE_RANGE_CHECK (pe, 2);                                 \
        return shmemi_comms_finc_request_##Name (target,        \
                                                 pe);           \
    }

SHMEM_TYPE_FINC (int, int);
SHMEM_TYPE_FINC (long, long);
SHMEM_TYPE_FINC (longlong, long long);

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_int_add = pshmem_int_add
#define shmem_int_add pshmem_int_add
#pragma weak shmem_long_add = pshmem_long_add
#define shmem_long_add pshmem_long_add
#pragma weak shmem_longlong_add = pshmem_longlong_add
#define shmem_longlong_add pshmem_longlong_add
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * remote atomic increment/add
 *
 */
#define SHMEM_TYPE_ADD(Name, Type)                                      \
    void                                                                \
    shmem_##Name##_add (Type *target, Type value, int pe)               \
    {                                                                   \
        INIT_CHECK ();                                                  \
        PE_RANGE_CHECK (pe, 3);                                         \
        shmemi_comms_add_request_##Name (target,                        \
                                         value,                         \
                                         pe);                           \
    }

SHMEM_TYPE_ADD (int, int);
SHMEM_TYPE_ADD (long, long);
SHMEM_TYPE_ADD (longlong, long long);


#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_int_inc = pshmem_int_inc
#define shmem_int_inc pshmem_int_inc
#pragma weak shmem_long_inc = pshmem_long_inc
#define shmem_long_inc pshmem_long_inc
#pragma weak shmem_longlong_inc = pshmem_longlong_inc
#define shmem_longlong_inc pshmem_longlong_inc
#endif /* HAVE_FEATURE_PSHMEM */

#define SHMEM_TYPE_INC(Name, Type)                              \
    void                                                        \
    shmem_##Name##_inc (Type *target, int pe)                   \
    {                                                           \
        INIT_CHECK ();                                          \
        PE_RANGE_CHECK (pe, 2);                                 \
        shmemi_comms_inc_request_##Name (target,                \
                                         pe);                   \
    }

SHMEM_TYPE_INC (int, int);
SHMEM_TYPE_INC (long, long);
SHMEM_TYPE_INC (longlong, long long);

/* --------------------------------------------------------------- */

#if defined(HAVE_FEATURE_EXPERIMENTAL)

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmemx_int_xor = pshmemx_int_xor
#define shmemx_int_xor pshmemx_int_xor
#pragma weak shmemx_long_xor = pshmemx_long_xor
#define shmemx_long_xor pshmemx_long_xor
#pragma weak shmemx_longlong_xor = pshmemx_longlong_xor
#define shmemx_longlong_xor pshmemx_longlong_xor
#endif /* HAVE_FEATURE_PSHMEM */

#define SHMEMX_TYPE_XOR(Name, Type)                                     \
    void                                                                \
    shmemx_##Name##_xor (Type *target, Type value, int pe)              \
    {                                                                   \
        INIT_CHECK ();                                                  \
        PE_RANGE_CHECK (pe, 3);                                         \
        shmemi_comms_xor_request_##Name (target, value,                 \
                                         pe);                           \
    }

SHMEMX_TYPE_XOR (int, int);
SHMEMX_TYPE_XOR (long, long);
SHMEMX_TYPE_XOR (longlong, long long);



#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmemx_int_fetch = pshmemx_int_fetch
#define shmemx_int_fetch pshmemx_int_fetch
#pragma weak shmemx_long_fetch = pshmemx_long_fetch
#define shmemx_long_fetch pshmemx_long_fetch
#pragma weak shmemx_longlong_fetch = pshmemx_longlong_fetch
#define shmemx_longlong_fetch pshmemx_longlong_fetch
#endif /* HAVE_FEATURE_PSHMEM */

#define SHMEM_TYPE_FETCH(Name, Type)                              \
    Type                                                          \
    shmemx_##Name##_fetch (Type *target, int pe)                  \
    {                                                             \
        INIT_CHECK ();                                            \
        PE_RANGE_CHECK (pe, 2);                                   \
        return shmemi_comms_fetch_request_##Name (target,         \
                                                  pe);            \
    }

SHMEM_TYPE_FETCH (int, int);
SHMEM_TYPE_FETCH (long, long);
SHMEM_TYPE_FETCH (longlong, long long);


#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmemx_int_set = pshmemx_int_set
#define shmemx_int_set pshmemx_int_set
#pragma weak shmemx_long_set = pshmemx_long_set
#define shmemx_long_set pshmemx_long_set
#pragma weak shmemx_longlong_set = pshmemx_longlong_set
#define shmemx_longlong_set pshmemx_longlong_set
#endif /* HAVE_FEATURE_PSHMEM */

#define SHMEM_TYPE_SET(Name, Type)                                \
    void                                                          \
    shmemx_##Name##_set (Type *target, Type value, int pe)        \
    {                                                             \
        INIT_CHECK ();                                            \
        PE_RANGE_CHECK (pe, 2);                                   \
        shmemi_comms_set_request_##Name (target, value,           \
                                         pe);                     \
    }

SHMEM_TYPE_SET (int, int);
SHMEM_TYPE_SET (long, long);
SHMEM_TYPE_SET (longlong, long long);

#endif /* HAVE_FEATURE_EXPERIMENTAL */
