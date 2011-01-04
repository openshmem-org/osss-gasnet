#include "state.h"
#include "warn.h"

#include "shmem.h"

/*
 * these are the arithmetic operations
 *
 */

#define SHMEM_MATH_FUNC(Name, Type)		\
  static inline Type				\
  sum_##Name##_func(Type a, Type b)		\
  {						\
    return (a) + (b);				\
  }						\
  static inline Type				\
  prod_##Name##_func(Type a, Type b)		\
  {						\
    return (a) * (b);				\
  }

SHMEM_MATH_FUNC(short, short)
SHMEM_MATH_FUNC(int, int)
SHMEM_MATH_FUNC(long, long)
SHMEM_MATH_FUNC(double, double)
SHMEM_MATH_FUNC(float, float)
SHMEM_MATH_FUNC(longlong, long long)
SHMEM_MATH_FUNC(longdouble, long double)
SHMEM_MATH_FUNC(complexd, double complex)
SHMEM_MATH_FUNC(complexf, float complex)

/*
 * these are the logical operations
 *
 */

#define SHMEM_LOGIC_FUNC(Name, Type)		\
  static inline Type				\
  and_##Name##_func(Type a, Type b)		\
  {						\
    return (a) && (b);				\
  }						\
  static inline Type				\
  or_##Name##_func(Type a, Type b)		\
  {						\
    return (a) || (b);				\
  }						\
  static inline Type				\
  xor_##Name##_func(Type a, Type b)		\
  {						\
    return (a) != (b);				\
  }

SHMEM_LOGIC_FUNC(short, short)
SHMEM_LOGIC_FUNC(int, int)
SHMEM_LOGIC_FUNC(long, long)
SHMEM_LOGIC_FUNC(longlong, long long)

/*
 * these are the minima/maxima operations
 *
 */

#define SHMEM_MINIMAX_FUNC(Name, Type)		\
  static inline Type				\
  min_##Name##_func(Type a, Type b)		\
  {						\
    return (a) < (b) ? (a) : (b);		\
  }						\
  static inline Type				\
  max_##Name##_func(Type a, Type b)		\
  {						\
    return (a) > (b) ? (a) : (b);		\
  }

SHMEM_MINIMAX_FUNC(short, short)
SHMEM_MINIMAX_FUNC(int, int)
SHMEM_MINIMAX_FUNC(long, long)
SHMEM_MINIMAX_FUNC(longlong, long long)
SHMEM_MINIMAX_FUNC(double, double)
SHMEM_MINIMAX_FUNC(float, float)
SHMEM_MINIMAX_FUNC(longdouble, long double)

/*
 * common reduce code.  Pass in type/operation, macro builds the
 * reduction function as defined above
 *
 */

#define SHMEM_REDUCE_TYPE_OP(Name, Type, OpCall)			\
  /* @api@ */								\
  void									\
  shmem_##Name##_##OpCall##_to_all(Type *target, Type *source, int nreduce, \
				   int PE_start, int logPE_stride, int PE_size, \
				   Type *pWrk, long *pSync)		\
  {									\
    const int step = 1 << logPE_stride;					\
    const int nloops = nreduce / _SHMEM_REDUCE_SYNC_SIZE;		\
    const int nrem = nreduce % _SHMEM_REDUCE_SYNC_SIZE;			\
    size_t nget = _SHMEM_REDUCE_SYNC_SIZE * sizeof(Type);		\
    int i, j;								\
    int pe;								\
    /* init target with own source, and wait for all */			\
    for (j = 0; j < nreduce; j += 1 ) {					\
      target[j] = source[j];						\
    }									\
    shmem_barrier(PE_start, logPE_stride, PE_size, pSync);		\
    /* now go through other PEs and get source */			\
    pe = PE_start;							\
    for (i = 0; i < PE_size; i+= 1) {					\
      if (__state.mype != pe) {						\
	int k;								\
	int ti = 0, si = 0; /* target and source index walk */		\
	/* pull in all the full chunks */				\
	for (k = 0; k < nloops; k += 1) {				\
	  shmem_getmem(pWrk, & (source[si]), nget, pe);			\
	  for (j = 0; j < _SHMEM_REDUCE_SYNC_SIZE; j += 1) {		\
	    target[ti] = OpCall##_##Name##_func(target[ti], pWrk[j]);	\
	    ti += 1;							\
	  }								\
	  si += _SHMEM_REDUCE_SYNC_SIZE;				\
	}								\
	nget = nrem * sizeof(Type);					\
	/* now get remaining part of source */				\
	shmem_getmem(pWrk, & (source[si]), nget, pe);			\
	for (j = 0; j < nrem; j += 1) {					\
	  target[ti] = OpCall##_##Name##_func(target[ti], pWrk[j]);	\
	  ti += 1;							\
	}								\
      }									\
      pe += step;							\
    }									\
  }
  
SHMEM_REDUCE_TYPE_OP(short, short, sum)
SHMEM_REDUCE_TYPE_OP(int, int, sum)
SHMEM_REDUCE_TYPE_OP(long, long, sum)
SHMEM_REDUCE_TYPE_OP(longlong, long long, sum)
SHMEM_REDUCE_TYPE_OP(double, double, sum)
SHMEM_REDUCE_TYPE_OP(float, float, sum)
SHMEM_REDUCE_TYPE_OP(longdouble, long double, sum)
SHMEM_REDUCE_TYPE_OP(complexd, double complex, sum)
SHMEM_REDUCE_TYPE_OP(complexf, float complex, sum)
  
SHMEM_REDUCE_TYPE_OP(short, short, prod)
SHMEM_REDUCE_TYPE_OP(int, int, prod)
SHMEM_REDUCE_TYPE_OP(long, long, prod)
SHMEM_REDUCE_TYPE_OP(longlong, long long, prod)
SHMEM_REDUCE_TYPE_OP(double, double, prod)
SHMEM_REDUCE_TYPE_OP(float, float, prod)
SHMEM_REDUCE_TYPE_OP(longdouble, long double, prod)
SHMEM_REDUCE_TYPE_OP(complexd, double complex, prod)
SHMEM_REDUCE_TYPE_OP(complexf, float complex, prod)
  
SHMEM_REDUCE_TYPE_OP(short, short, and)
SHMEM_REDUCE_TYPE_OP(int, int, and)
SHMEM_REDUCE_TYPE_OP(long, long, and)
SHMEM_REDUCE_TYPE_OP(longlong, long long, and)
  
SHMEM_REDUCE_TYPE_OP(short, short, or)
SHMEM_REDUCE_TYPE_OP(int, int, or)
SHMEM_REDUCE_TYPE_OP(long, long, or)
SHMEM_REDUCE_TYPE_OP(longlong, long long, or)
  
SHMEM_REDUCE_TYPE_OP(short, short, xor)
SHMEM_REDUCE_TYPE_OP(int, int, xor)
SHMEM_REDUCE_TYPE_OP(long, long, xor)
SHMEM_REDUCE_TYPE_OP(longlong, long long, xor)
  
SHMEM_REDUCE_TYPE_OP(short, short, max)
SHMEM_REDUCE_TYPE_OP(int, int, max)
SHMEM_REDUCE_TYPE_OP(long, long, max)
SHMEM_REDUCE_TYPE_OP(longlong, long long, max)
SHMEM_REDUCE_TYPE_OP(double, double, max)
SHMEM_REDUCE_TYPE_OP(float, float, max)
SHMEM_REDUCE_TYPE_OP(longdouble, long double, max)
  
SHMEM_REDUCE_TYPE_OP(short, short, min)
SHMEM_REDUCE_TYPE_OP(int, int, min)
SHMEM_REDUCE_TYPE_OP(long, long, min)
SHMEM_REDUCE_TYPE_OP(longlong, long long, min)
SHMEM_REDUCE_TYPE_OP(double, double, min)
SHMEM_REDUCE_TYPE_OP(float, float, min)
SHMEM_REDUCE_TYPE_OP(longdouble, long double, min)
