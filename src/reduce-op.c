#include "state.h"
#include "trace.h"
#include "putget.h"
#include "comms.h"
#include "globalvar.h"
#include "utils.h"

#include "mpp/shmem.h"

/*
 * pre-defined reductions in SHMEM 1.0
 *
 */

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
    return !(a) ^ !(b);				\
  }

SHMEM_LOGIC_FUNC(short, short)
SHMEM_LOGIC_FUNC(int, int)
SHMEM_LOGIC_FUNC(long, long)
SHMEM_LOGIC_FUNC(longlong, long long)
SHMEM_LOGIC_FUNC(complexf, float complex)

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
 * common reduce code.  Build generalized reduction for various types.
 * Comparison operator passed as 1st parameter
 *
 */

#define SHMEM_UDR_TYPE_OP(Name, Type)					\
  static void								\
  __shmem_udr_##Name##_to_all(Type (*the_op)(Type, Type),		\
			      Type *target, Type *source, int nreduce,	\
			      int PE_start, int logPE_stride, int PE_size, \
			      Type *pWrk, long *pSync)			\
  {									\
    const int step = 1 << logPE_stride;					\
    const int nloops = nreduce / _SHMEM_REDUCE_SYNC_SIZE;		\
    const int nrem = nreduce % _SHMEM_REDUCE_SYNC_SIZE;			\
    size_t nget = _SHMEM_REDUCE_SYNC_SIZE * sizeof(Type);		\
    int i, j;								\
    int pe;								\
    /* init target with own source, and wait for all */			\
    for (j = 0; j < nreduce; j += 1) {					\
      target[j] = source[j];						\
    }									\
    shmem_barrier(PE_start, logPE_stride, PE_size, pSync);		\
    /* now go through other PEs and get source */			\
    pe = PE_start;							\
    for (i = 0; i < PE_size; i+= 1) {					\
      if (GET_STATE(mype) != pe) {					\
	int k;								\
	int ti = 0, si = 0; /* target and source index walk */		\
	/* pull in all the full chunks */				\
	for (k = 0; k < nloops; k += 1) {				\
	  shmem_getmem(pWrk, & (source[si]), nget, pe);			\
	  for (j = 0; j < _SHMEM_REDUCE_SYNC_SIZE; j += 1) {		\
	    target[ti] = (*the_op)(target[ti], pWrk[j]);		\
	    ti += 1;							\
	  }								\
	  si += _SHMEM_REDUCE_SYNC_SIZE;				\
	}								\
	nget = nrem * sizeof(Type);					\
	/* now get remaining part of source */				\
	shmem_getmem(pWrk, & (source[si]), nget, pe);			\
	for (j = 0; j < nrem; j += 1) {					\
	  target[ti] = (*the_op)(target[ti], pWrk[j]);			\
	  ti += 1;							\
	}								\
      }									\
      pe += step;							\
    }									\
  }

SHMEM_UDR_TYPE_OP(short, short)
SHMEM_UDR_TYPE_OP(int, int)
SHMEM_UDR_TYPE_OP(long, long)
SHMEM_UDR_TYPE_OP(longlong, long long)
SHMEM_UDR_TYPE_OP(double, double)
SHMEM_UDR_TYPE_OP(float, float)
SHMEM_UDR_TYPE_OP(longdouble, long double)
SHMEM_UDR_TYPE_OP(complexd, double complex)
SHMEM_UDR_TYPE_OP(complexf, float complex)

/*
 * Pass in type/operation, macro builds the reduction function as
 * defined above
 *
 */

#define SHMEM_REDUCE_TYPE_OP(OpCall, Name, Type)			\
  /* @api@ */								\
  void									\
  pshmem_##Name##_##OpCall##_to_all(Type *target, Type *source, int nreduce, \
				    int PE_start, int logPE_stride, int PE_size, \
				    Type *pWrk, long *pSync)		\
  {									\
									\
    INIT_CHECK();							\
    SYMMETRY_CHECK(target, 1, "shmem_" #Name "_" #OpCall "_to_all");	\
    SYMMETRY_CHECK(source, 2, "shmem_" #Name "_" #OpCall "_to_all");	\
    __shmem_udr_##Name##_to_all(OpCall##_##Name##_func,			\
				target, source, nreduce,		\
				PE_start, logPE_stride, PE_size,	\
				pWrk, pSync);				\
  }


SHMEM_REDUCE_TYPE_OP(sum, short, short)
SHMEM_REDUCE_TYPE_OP(sum, int, int)
SHMEM_REDUCE_TYPE_OP(sum, long, long)
SHMEM_REDUCE_TYPE_OP(sum, longlong, long long)
SHMEM_REDUCE_TYPE_OP(sum, double, double)
SHMEM_REDUCE_TYPE_OP(sum, float, float)
SHMEM_REDUCE_TYPE_OP(sum, longdouble, long double)
SHMEM_REDUCE_TYPE_OP(sum, complexd, double complex)
SHMEM_REDUCE_TYPE_OP(sum, complexf, float complex)
  
SHMEM_REDUCE_TYPE_OP(prod, short, short)
SHMEM_REDUCE_TYPE_OP(prod, int, int)
SHMEM_REDUCE_TYPE_OP(prod, long, long)
SHMEM_REDUCE_TYPE_OP(prod, longlong, long long)
SHMEM_REDUCE_TYPE_OP(prod, double, double)
SHMEM_REDUCE_TYPE_OP(prod, float, float)
SHMEM_REDUCE_TYPE_OP(prod, longdouble, long double)
SHMEM_REDUCE_TYPE_OP(prod, complexd, double complex)
SHMEM_REDUCE_TYPE_OP(prod, complexf, float complex)
  
SHMEM_REDUCE_TYPE_OP(and, short, short)
SHMEM_REDUCE_TYPE_OP(and, int, int)
SHMEM_REDUCE_TYPE_OP(and, long, long)
SHMEM_REDUCE_TYPE_OP(and, longlong, long long)

SHMEM_REDUCE_TYPE_OP(or, short, short)
SHMEM_REDUCE_TYPE_OP(or, int, int)
SHMEM_REDUCE_TYPE_OP(or, long, long)
SHMEM_REDUCE_TYPE_OP(or, longlong, long long)

SHMEM_REDUCE_TYPE_OP(xor, short, short)
SHMEM_REDUCE_TYPE_OP(xor, int, int)
SHMEM_REDUCE_TYPE_OP(xor, long, long)
SHMEM_REDUCE_TYPE_OP(xor, longlong, long long)
SHMEM_REDUCE_TYPE_OP(xor, complexf, float complex)

SHMEM_REDUCE_TYPE_OP(max, short, short)
SHMEM_REDUCE_TYPE_OP(max, int, int)
SHMEM_REDUCE_TYPE_OP(max, long, long)
SHMEM_REDUCE_TYPE_OP(max, longlong, long long)
SHMEM_REDUCE_TYPE_OP(max, double, double)
SHMEM_REDUCE_TYPE_OP(max, float, float)
SHMEM_REDUCE_TYPE_OP(max, longdouble, long double)
  
SHMEM_REDUCE_TYPE_OP(min, short, short)
SHMEM_REDUCE_TYPE_OP(min, int, int)
SHMEM_REDUCE_TYPE_OP(min, long, long)
SHMEM_REDUCE_TYPE_OP(min, longlong, long long)
SHMEM_REDUCE_TYPE_OP(min, double, double)
SHMEM_REDUCE_TYPE_OP(min, float, float)
SHMEM_REDUCE_TYPE_OP(min, longdouble, long double)

#pragma weak shmem_complexd_sum_to_all = pshmem_complexd_sum_to_all
#pragma weak shmem_complexf_sum_to_all = pshmem_complexf_sum_to_all
#pragma weak shmem_double_sum_to_all = pshmem_double_sum_to_all
#pragma weak shmem_float_sum_to_all = pshmem_float_sum_to_all
#pragma weak shmem_int_sum_to_all = pshmem_int_sum_to_all
#pragma weak shmem_long_sum_to_all = pshmem_long_sum_to_all
#pragma weak shmem_longdouble_sum_to_all = pshmem_longdouble_sum_to_all
#pragma weak shmem_longlong_sum_to_all = pshmem_longlong_sum_to_all
#pragma weak shmem_short_sum_to_all = pshmem_short_sum_to_all

#pragma weak shmem_complexd_prod_to_all = pshmem_complexd_prod_to_all
#pragma weak shmem_complexf_prod_to_all = pshmem_complexf_prod_to_all
#pragma weak shmem_double_prod_to_all = pshmem_double_prod_to_all
#pragma weak shmem_float_prod_to_all = pshmem_float_prod_to_all
#pragma weak shmem_int_prod_to_all = pshmem_int_prod_to_all
#pragma weak shmem_long_prod_to_all = pshmem_long_prod_to_all
#pragma weak shmem_longdouble_prod_to_all = pshmem_longdouble_prod_to_all
#pragma weak shmem_longlong_prod_to_all = pshmem_longlong_prod_to_all
#pragma weak shmem_short_prod_to_all = pshmem_short_prod_to_all

#pragma weak shmem_int_and_to_all = pshmem_int_and_to_all
#pragma weak shmem_long_and_to_all = pshmem_long_and_to_all
#pragma weak shmem_longlong_and_to_all = pshmem_longlong_and_to_all
#pragma weak shmem_short_and_to_all = pshmem_short_and_to_all

#pragma weak shmem_int_or_to_all = pshmem_int_or_to_all
#pragma weak shmem_long_or_to_all = pshmem_long_or_to_all
#pragma weak shmem_longlong_or_to_all = pshmem_longlong_or_to_all
#pragma weak shmem_short_or_to_all = pshmem_short_or_to_all

#pragma weak shmem_int_xor_to_all = pshmem_int_xor_to_all
#pragma weak shmem_long_xor_to_all = pshmem_long_xor_to_all
#pragma weak shmem_longlong_xor_to_all = pshmem_longlong_xor_to_all
#pragma weak shmem_short_xor_to_all = pshmem_short_xor_to_all
#pragma weak shmem_complexf_xor_to_all = pshmem_complexf_xor_to_all

#pragma weak shmem_int_max_to_all = pshmem_int_max_to_all
#pragma weak shmem_long_max_to_all = pshmem_long_max_to_all
#pragma weak shmem_longlong_max_to_all = pshmem_longlong_max_to_all
#pragma weak shmem_short_max_to_all = pshmem_short_max_to_all
#pragma weak shmem_longdouble_max_to_all = pshmem_longdouble_max_to_all
#pragma weak shmem_float_max_to_all = pshmem_float_max_to_all
#pragma weak shmem_double_max_to_all = pshmem_double_max_to_all

#pragma weak shmem_int_min_to_all = pshmem_int_min_to_all
#pragma weak shmem_long_min_to_all = pshmem_long_min_to_all
#pragma weak shmem_longlong_min_to_all = pshmem_longlong_min_to_all
#pragma weak shmem_short_min_to_all = pshmem_short_min_to_all
#pragma weak shmem_longdouble_min_to_all = pshmem_longdouble_min_to_all
#pragma weak shmem_float_min_to_all = pshmem_float_min_to_all
#pragma weak shmem_double_min_to_all = pshmem_double_min_to_all
