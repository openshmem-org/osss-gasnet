/* (c) 2011 University of Houston.  All rights reserved. */


#include "trace.h"
#include "utils.h"

#include "mpp/shmem.h"

/*
 *
 * void shmem_int_iput(symmetric int *target, const int *source,
 *                     ptrdiff_t target_stride,
 *	   	       ptrdiff_t source_stride,
 *                     size_t n_elems, int pe);
 *
 */

#define SHMEM_EMIT_IPUT(Name, Type)					\
  /* @api@ */								\
  void									\
  pshmem_##Name##_iput(Type *target, const Type *source,		\
		       ptrdiff_t tst, ptrdiff_t sst, size_t nelems, int pe) \
  {									\
    size_t ti = 0, si = 0;						\
    size_t i;								\
    INIT_CHECK();							\
    PE_RANGE_CHECK(pe);							\
    for (i = 0; i < nelems; i += 1) {					\
      shmem_##Name##_p(& (target[ti]), source[si], pe);			\
      ti += tst;							\
      si += sst;							\
    }									\
  }

SHMEM_EMIT_IPUT(short, short)
SHMEM_EMIT_IPUT(int, int)
SHMEM_EMIT_IPUT(long, long)
SHMEM_EMIT_IPUT(double, double)
SHMEM_EMIT_IPUT(float, float)
SHMEM_EMIT_IPUT(longdouble, long double)
SHMEM_EMIT_IPUT(longlong, long long)

#pragma weak pshmem_iput32 = pshmem_int_iput
#pragma weak pshmem_iput64 = pshmem_long_iput
#pragma weak pshmem_iput128 = pshmem_longdouble_iput

#pragma weak shmem_short_iput = pshmem_short_iput
#pragma weak shmem_int_iput = pshmem_int_iput
#pragma weak shmem_long_iput = pshmem_long_iput
#pragma weak shmem_double_iput = pshmem_double_iput
#pragma weak shmem_longdouble_iput = pshmem_longdouble_iput
#pragma weak shmem_longlong_iput = pshmem_longlong_iput
#pragma weak shmem_iput32 = pshmem_iput32
#pragma weak shmem_iput64 = pshmem_iput64
#pragma weak shmem_iput128 = pshmem_iput128




/*
 *
 * void shmem_int_iget(int *target, symmetric const int *source,
 *                     ptrdiff_t target_stride,
 *	   	       ptrdiff_t source_stride,
 *                     size_t n_elems, int pe);
 *
 */

#define SHMEM_EMIT_IGET(Name, Type)					\
  /* @api@ */								\
  void									\
  pshmem_##Name##_iget(Type *target, const Type *source,		\
		       ptrdiff_t tst, ptrdiff_t sst, size_t nelems, int pe) \
  {									\
    size_t ti = 0, si = 0;						\
    size_t i;								\
    INIT_CHECK();							\
    PE_RANGE_CHECK(pe);							\
    for (i = 0; i < nelems; i += 1) {					\
      target[ti] = shmem_##Name##_g((Type *) & (source[si]), pe);	\
      ti += tst;							\
      si += sst;							\
    }									\
  }

SHMEM_EMIT_IGET(short, short)
SHMEM_EMIT_IGET(int, int)
SHMEM_EMIT_IGET(long, long)
SHMEM_EMIT_IGET(double, double)
SHMEM_EMIT_IGET(float, float)
SHMEM_EMIT_IGET(longdouble, long double)
SHMEM_EMIT_IGET(longlong, long long)

#pragma weak pshmem_iget32 = pshmem_int_iget
#pragma weak pshmem_iget64 = pshmem_long_iget
#pragma weak pshmem_iget128 = pshmem_longdouble_iget

#pragma weak shmem_short_iget = pshmem_short_iget
#pragma weak shmem_int_iget = pshmem_int_iget
#pragma weak shmem_long_iget = pshmem_long_iget
#pragma weak shmem_double_iget = pshmem_double_iget
#pragma weak shmem_longdouble_iget = pshmem_longdouble_iget
#pragma weak shmem_longlong_iget = pshmem_longlong_iget
#pragma weak shmem_iget32 = pshmem_iget32
#pragma weak shmem_iget64 = pshmem_iget64
#pragma weak shmem_iget128 = pshmem_iget128
