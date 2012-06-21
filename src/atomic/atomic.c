/*
 *
 * Copyright (c) 2011, 2012
 *   University of Houston System and Oak Ridge National Laboratory.
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
 * o Neither the name of the University of Houston System, Oak Ridge
 *   National Laboratory nor the names of its contributors may be used to
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
#include "comms.h"
#include "utils.h"
#include "atomic.h"

#include "shmem.h"

/*
 * placeholders: no init/final required (so far)
 */

void
__shmem_atomic_init (void)
{
}

void
__shmem_atomic_finalize (void)
{
}

/*
 * shmem_swap performs an atomic swap operation. It writes value
 * "value" into target on processing element (PE) pe and returns the
 * previous contents of target as an atomic operation.
 */

#define SHMEM_TYPE_SWAP(Name, Type)					\
  /* @api@ */								\
  Type									\
  pshmem_##Name##_swap(Type *target, Type value, int pe)		\
  {									\
    Type retval;							\
    INIT_CHECK();							\
    PE_RANGE_CHECK(pe);							\
    __shmem_comms_swap_request(target, &value, sizeof(Type), pe, &retval); \
    return retval;							\
  }

/* SHMEM_TYPE_SWAP(short, short) */
SHMEM_TYPE_SWAP (int, int);
SHMEM_TYPE_SWAP (long, long);
SHMEM_TYPE_SWAP (longlong, long long);
SHMEM_TYPE_SWAP (double, double);
SHMEM_TYPE_SWAP (float, float);

#pragma weak shmem_int_swap = pshmem_int_swap
#pragma weak shmem_long_swap = pshmem_long_swap
#pragma weak shmem_longlong_swap = pshmem_longlong_swap
#pragma weak shmem_float_swap = pshmem_float_swap
#pragma weak shmem_double_swap = pshmem_double_swap
#pragma weak pshmem_swap = pshmem_long_swap
#pragma weak shmem_swap = pshmem_long_swap

/*
 * The conditional swap routines conditionally update a target data
 * object on an arbitrary processing element (PE) and return the prior
 * contents of the data object in one atomic operation.
 */

#define SHMEM_TYPE_CSWAP(Name, Type)					\
  /* @api@ */								\
  Type									\
  pshmem_##Name##_cswap(Type *target, Type cond, Type value, int pe)	\
  {									\
    Type retval;							\
    INIT_CHECK();							\
    PE_RANGE_CHECK(pe);							\
    __shmem_comms_cswap_request(target, &cond, &value, sizeof(Type), pe, &retval); \
    return retval;							\
  }

SHMEM_TYPE_CSWAP (int, int);
SHMEM_TYPE_CSWAP (long, long);
SHMEM_TYPE_CSWAP (longlong, long long);

#pragma weak shmem_int_cswap = pshmem_int_cswap
#pragma weak shmem_long_cswap = pshmem_long_cswap
#pragma weak shmem_longlong_cswap = pshmem_longlong_cswap
#pragma weak pshmem_cswap = pshmem_long_cswap
/* not currently in SGI API #pragma weak shmem_cswap = pshmem_long_cswap */

#define SHMEM_TYPE_FADD(Name, Type)					\
  /* @api@ */								\
  Type									\
  pshmem_##Name##_fadd(Type *target, Type value, int pe)		\
  {									\
    Type retval;							\
    INIT_CHECK();							\
    PE_RANGE_CHECK(pe);							\
  __shmem_comms_fadd_request(target, &value, sizeof(Type), pe, &retval); \
    return retval;							\
  }

SHMEM_TYPE_FADD (int, int);
SHMEM_TYPE_FADD (long, long);
SHMEM_TYPE_FADD (longlong, long long);

#pragma weak shmem_int_fadd = pshmem_int_fadd
#pragma weak shmem_long_fadd = pshmem_long_fadd
#pragma weak shmem_longlong_fadd = pshmem_longlong_fadd

#define SHMEM_TYPE_FINC(Name, Type)					\
  /* @api@ */								\
  Type									\
  pshmem_##Name##_finc(Type *target, int pe)				\
  {									\
    Type retval;							\
    INIT_CHECK();							\
    PE_RANGE_CHECK(pe);							\
    __shmem_comms_finc_request(target, sizeof(Type), pe, &retval);	\
    return retval;							\
  }

SHMEM_TYPE_FINC (int, int);
SHMEM_TYPE_FINC (long, long);
SHMEM_TYPE_FINC (longlong, long long);

#pragma weak shmem_int_finc = pshmem_int_finc
#pragma weak shmem_long_finc = pshmem_long_finc
#pragma weak shmem_longlong_finc = pshmem_longlong_finc

/*
 * remote increment/add
 *
 */
#define SHMEM_TYPE_ADD(Name, Type)					\
  /* @api@ */								\
  void									\
  pshmem_##Name##_add(Type *target, Type value, int pe)			\
  {									\
    INIT_CHECK();							\
    PE_RANGE_CHECK(pe);							\
    __shmem_comms_add_request(target, &value, sizeof(Type), pe);	\
  }

SHMEM_TYPE_ADD (int, int);
SHMEM_TYPE_ADD (long, long);
SHMEM_TYPE_ADD (longlong, long long);

#pragma weak shmem_int_add = pshmem_int_add
#pragma weak shmem_long_add = pshmem_long_add
#pragma weak shmem_longlong_add = pshmem_longlong_add

#define SHMEM_TYPE_INC(Name, Type)					\
  /* @api@ */								\
  void									\
  pshmem_##Name##_inc(Type *target, int pe)				\
  {									\
    INIT_CHECK();							\
    PE_RANGE_CHECK(pe);							\
    __shmem_comms_inc_request(target, sizeof(Type), pe);		\
  }

SHMEM_TYPE_INC (int, int);
SHMEM_TYPE_INC (long, long);
SHMEM_TYPE_INC (longlong, long long);

#pragma weak shmem_int_inc = pshmem_int_inc
#pragma weak shmem_long_inc = pshmem_long_inc
#pragma weak shmem_longlong_inc = pshmem_longlong_inc
