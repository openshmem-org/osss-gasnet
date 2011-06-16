/* (c) 2011 University of Houston.  All rights reserved. */


#include <sys/types.h>

#include "state.h"
#include "comms.h"
#include "utils.h"
#include "atomic.h"

#include "mpp/shmem.h"

/*
 * placeholders: no init/final required (so far)
 */

void
__shmem_atomic_init(void)
{
}

void
__shmem_atomic_finalize(void)
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
SHMEM_TYPE_SWAP(int, int)
SHMEM_TYPE_SWAP(long, long)
SHMEM_TYPE_SWAP(longlong, long long)
SHMEM_TYPE_SWAP(double, double)
SHMEM_TYPE_SWAP(float, float)

#pragma weak pshmem_swap = pshmem_long_swap

#pragma weak shmem_int_swap = pshmem_int_swap
#pragma weak shmem_long_swap = pshmem_long_swap
#pragma weak shmem_longlong_swap = pshmem_longlong_swap
#pragma weak shmem_float_swap = pshmem_float_swap
#pragma weak shmem_double_swap = pshmem_double_swap
#pragma weak shmem_swap = pshmem_swap


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

SHMEM_TYPE_CSWAP(int, int)
SHMEM_TYPE_CSWAP(long, long)
SHMEM_TYPE_CSWAP(longlong, long long)

#pragma weak pshmem_cswap = pshmem_long_cswap

#pragma weak shmem_int_cswap = pshmem_int_cswap
#pragma weak shmem_long_cswap = pshmem_long_cswap
#pragma weak shmem_longlong_cswap = pshmem_longlong_cswap
#pragma weak shmem_cswap = pshmem_cswap

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
  
SHMEM_TYPE_FADD(int, int)
SHMEM_TYPE_FADD(long, long)
SHMEM_TYPE_FADD(longlong, long long)

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

SHMEM_TYPE_FINC(int, int)
SHMEM_TYPE_FINC(long, long)
SHMEM_TYPE_FINC(longlong, long long)

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
  
SHMEM_TYPE_ADD(int, int)
SHMEM_TYPE_ADD(long, long)
SHMEM_TYPE_ADD(longlong, long long)

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

SHMEM_TYPE_INC(int, int)
SHMEM_TYPE_INC(long, long)
SHMEM_TYPE_INC(longlong, long long)

#pragma weak shmem_int_inc = pshmem_int_inc
#pragma weak shmem_long_inc = pshmem_long_inc
#pragma weak shmem_longlong_inc = pshmem_longlong_inc
