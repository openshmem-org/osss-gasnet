/*
 *
 * Copyright (c) 2011 - 2013
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



#include "state.h"
#include "trace.h"
#include "putget.h"
#include "globalvar.h"
#include "utils.h"
#include "atomic.h"

#include "shmem.h"

/**
 * pre-defined reductions in SHMEM 1.0
 *
 */

/**
 * these are the arithmetic operations
 *
 */

#define SHMEM_MATH_FUNC(Name, Type)		\
  static Type					\
  sum_##Name##_func(Type a, Type b)		\
  {						\
    return (a) + (b);				\
  }						\
  static Type					\
  prod_##Name##_func(Type a, Type b)		\
  {						\
    return (a) * (b);				\
  }

SHMEM_MATH_FUNC (short, short);
SHMEM_MATH_FUNC (int, int);
SHMEM_MATH_FUNC (long, long);
SHMEM_MATH_FUNC (double, double);
SHMEM_MATH_FUNC (float, float);
SHMEM_MATH_FUNC (longlong, long long);
SHMEM_MATH_FUNC (longdouble, long double);
SHMEM_MATH_FUNC (complexd, double complex);
SHMEM_MATH_FUNC (complexf, float complex);

/**
 * these are the logical operations.  Note: these are *bitwise*.
 *
 */

#define SHMEM_LOGIC_FUNC(Name, Type)		\
  static Type					\
  and_##Name##_func(Type a, Type b)		\
  {						\
    return (a) & (b);				\
  }						\
  static Type					\
  or_##Name##_func(Type a, Type b)		\
  {						\
    return (a) | (b);				\
  }						\
  static Type					\
  xor_##Name##_func(Type a, Type b)		\
  {						\
    return (a) ^ (b);				\
  }

SHMEM_LOGIC_FUNC (short, short);
SHMEM_LOGIC_FUNC (int, int);
SHMEM_LOGIC_FUNC (long, long);
SHMEM_LOGIC_FUNC (longlong, long long);

/**
 * these are the minima/maxima operations
 *
 */

#define SHMEM_MINIMAX_FUNC(Name, Type)		\
  static Type					\
  min_##Name##_func(Type a, Type b)		\
  {						\
    return (a) < (b) ? (a) : (b);		\
  }						\
  static Type					\
  max_##Name##_func(Type a, Type b)		\
  {						\
    return (a) > (b) ? (a) : (b);		\
  }

SHMEM_MINIMAX_FUNC (short, short);
SHMEM_MINIMAX_FUNC (int, int);
SHMEM_MINIMAX_FUNC (long, long);
SHMEM_MINIMAX_FUNC (longlong, long long);
SHMEM_MINIMAX_FUNC (double, double);
SHMEM_MINIMAX_FUNC (float, float);
SHMEM_MINIMAX_FUNC (longdouble, long double);

/**
 * common reduce code.  Build generalized reduction for various types.
 * Comparison operator passed as 1st parameter
 *
 */

#include <string.h>
#include <stdlib.h>

#define OVERLAP_CHECK(t, s, n) ( (t) >= (s) ) && ( (t) < ( (s) + (n) ) )

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
    const int snred = sizeof(Type) * nreduce;				\
    const int overlap = OVERLAP_CHECK(target, source, snred);		\
    size_t nget;							\
    int i, j;								\
    int pe;								\
    Type *tmptrg = NULL;						\
    Type *write_to;							\
    if (overlap) {							\
      /* use temp target in case source/target overlap/same */		\
      tmptrg = (Type *) malloc(snred);					\
      if (tmptrg == (Type *) NULL) {					\
	__shmem_trace(SHMEM_LOG_FATAL,					\
		      "internal error: out of memory allocating temporary reduction buffer" \
		      );						\
      }									\
      write_to = tmptrg;						\
      __shmem_trace(SHMEM_LOG_REDUCE,					\
		    "target (%p) and source (%p, size %ld) overlap, using temporary target", \
		    target, source, snred				\
		    );							\
    }									\
    else {								\
      write_to = target;						\
      __shmem_trace(SHMEM_LOG_REDUCE,					\
		    "target (%p) and source (%p, size %ld) do not overlap", \
		    target, source, snred				\
		    );							\
    } /* end overlap check */						\
    /* everyone must initialize */					\
    for (j = 0; j < nreduce; j += 1) {					\
      write_to[j] = source[j];						\
    }									\
    shmem_barrier(PE_start, logPE_stride, PE_size, pSync);		\
    __shmem_trace(SHMEM_LOG_REDUCE,					\
		  "after first barrier"					\
		  );							\
    /* now go through other PEs and get source */			\
    pe = PE_start;							\
    for (i = 0; i < PE_size; i+= 1) {					\
      if (GET_STATE(mype) != pe) {					\
	int k;								\
	int ti = 0, si = 0; /* target and source index walk */		\
	/* pull in all the full chunks */				\
	nget = _SHMEM_REDUCE_SYNC_SIZE * sizeof(Type);			\
	for (k = 0; k < nloops; k += 1) {				\
	  shmem_getmem(pWrk, & (source[si]), nget, pe);			\
	  for (j = 0; j < _SHMEM_REDUCE_SYNC_SIZE; j += 1) {		\
	    write_to[ti] = (*the_op)(write_to[ti], pWrk[j]);		\
	    ti += 1;							\
	  }								\
	  si += _SHMEM_REDUCE_SYNC_SIZE;				\
	}								\
	nget = nrem * sizeof(Type);					\
	/* now get remaining part of source */				\
	shmem_getmem(pWrk, & (source[si]), nget, pe);			\
	for (j = 0; j < nrem; j += 1) {					\
	  write_to[ti] = (*the_op)(write_to[ti], pWrk[j]);		\
	  ti += 1;							\
	}								\
      }									\
      pe += step;							\
    }									\
    /* everyone has to have finished */					\
    shmem_barrier(PE_start, logPE_stride, PE_size, pSync);		\
    if (overlap) {							\
      /* write to real local target and free temp */			\
      memcpy(target, tmptrg, snred);					\
      free(tmptrg);							\
      tmptrg = NULL;							\
      /* shmem_barrier(PE_start, logPE_stride, PE_size, pSync);	 */	\
      shmem_quiet ();							\
    }									\
  }

SHMEM_UDR_TYPE_OP (short, short);
SHMEM_UDR_TYPE_OP (int, int);
SHMEM_UDR_TYPE_OP (long, long);
SHMEM_UDR_TYPE_OP (longlong, long long);
SHMEM_UDR_TYPE_OP (double, double);
SHMEM_UDR_TYPE_OP (float, float);
SHMEM_UDR_TYPE_OP (longdouble, long double);
SHMEM_UDR_TYPE_OP (complexd, double complex);
SHMEM_UDR_TYPE_OP (complexf, float complex);




#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_complexd_sum_to_all = pshmem_complexd_sum_to_all
#define shmem_complexd_sum_to_all pshmem_complexd_sum_to_all
#pragma weak shmem_complexf_sum_to_all = pshmem_complexf_sum_to_all
#define shmem_complexf_sum_to_all pshmem_complexf_sum_to_all
#pragma weak shmem_double_sum_to_all = pshmem_double_sum_to_all
#define shmem_double_sum_to_all pshmem_double_sum_to_all
#pragma weak shmem_float_sum_to_all = pshmem_float_sum_to_all
#define shmem_float_sum_to_all pshmem_float_sum_to_all
#pragma weak shmem_int_sum_to_all = pshmem_int_sum_to_all
#define shmem_int_sum_to_all pshmem_int_sum_to_all
#pragma weak shmem_long_sum_to_all = pshmem_long_sum_to_all
#define shmem_long_sum_to_all pshmem_long_sum_to_all
#pragma weak shmem_longdouble_sum_to_all = pshmem_longdouble_sum_to_all
#define shmem_longdouble_sum_to_all pshmem_longdouble_sum_to_all
#pragma weak shmem_longlong_sum_to_all = pshmem_longlong_sum_to_all
#define shmem_longlong_sum_to_all pshmem_longlong_sum_to_all
#pragma weak shmem_short_sum_to_all = pshmem_short_sum_to_all
#define shmem_short_sum_to_all pshmem_short_sum_to_all
#pragma weak shmem_complexd_prod_to_all = pshmem_complexd_prod_to_all
#define shmem_complexd_prod_to_all pshmem_complexd_prod_to_all
#pragma weak shmem_complexf_prod_to_all = pshmem_complexf_prod_to_all
#define shmem_complexf_prod_to_all pshmem_complexf_prod_to_all
#pragma weak shmem_double_prod_to_all = pshmem_double_prod_to_all
#define shmem_double_prod_to_all pshmem_double_prod_to_all
#pragma weak shmem_float_prod_to_all = pshmem_float_prod_to_all
#define shmem_float_prod_to_all pshmem_float_prod_to_all
#pragma weak shmem_int_prod_to_all = pshmem_int_prod_to_all
#define shmem_int_prod_to_all pshmem_int_prod_to_all
#pragma weak shmem_long_prod_to_all = pshmem_long_prod_to_all
#define shmem_long_prod_to_all pshmem_long_prod_to_all
#pragma weak shmem_longdouble_prod_to_all = pshmem_longdouble_prod_to_all
#define shmem_longdouble_prod_to_all pshmem_longdouble_prod_to_all
#pragma weak shmem_longlong_prod_to_all = pshmem_longlong_prod_to_all
#define shmem_longlong_prod_to_all pshmem_longlong_prod_to_all
#pragma weak shmem_short_prod_to_all = pshmem_short_prod_to_all
#define shmem_short_prod_to_all pshmem_short_prod_to_all
#pragma weak shmem_int_and_to_all = pshmem_int_and_to_all
#define shmem_int_and_to_all pshmem_int_and_to_all
#pragma weak shmem_long_and_to_all = pshmem_long_and_to_all
#define shmem_long_and_to_all pshmem_long_and_to_all
#pragma weak shmem_longlong_and_to_all = pshmem_longlong_and_to_all
#define shmem_longlong_and_to_all pshmem_longlong_and_to_all
#pragma weak shmem_short_and_to_all = pshmem_short_and_to_all
#define shmem_short_and_to_all pshmem_short_and_to_all
#pragma weak shmem_int_or_to_all = pshmem_int_or_to_all
#define shmem_int_or_to_all pshmem_int_or_to_all
#pragma weak shmem_long_or_to_all = pshmem_long_or_to_all
#define shmem_long_or_to_all pshmem_long_or_to_all
#pragma weak shmem_longlong_or_to_all = pshmem_longlong_or_to_all
#define shmem_longlong_or_to_all pshmem_longlong_or_to_all
#pragma weak shmem_short_or_to_all = pshmem_short_or_to_all
#define shmem_short_or_to_all pshmem_short_or_to_all
#pragma weak shmem_int_xor_to_all = pshmem_int_xor_to_all
#define shmem_int_xor_to_all pshmem_int_xor_to_all
#pragma weak shmem_long_xor_to_all = pshmem_long_xor_to_all
#define shmem_long_xor_to_all pshmem_long_xor_to_all
#pragma weak shmem_longlong_xor_to_all = pshmem_longlong_xor_to_all
#define shmem_longlong_xor_to_all pshmem_longlong_xor_to_all
#pragma weak shmem_short_xor_to_all = pshmem_short_xor_to_all
#define shmem_short_xor_to_all pshmem_short_xor_to_all
#pragma weak shmem_int_max_to_all = pshmem_int_max_to_all
#define shmem_int_max_to_all pshmem_int_max_to_all
#pragma weak shmem_long_max_to_all = pshmem_long_max_to_all
#define shmem_long_max_to_all pshmem_long_max_to_all
#pragma weak shmem_longlong_max_to_all = pshmem_longlong_max_to_all
#define shmem_longlong_max_to_all pshmem_longlong_max_to_all
#pragma weak shmem_short_max_to_all = pshmem_short_max_to_all
#define shmem_short_max_to_all pshmem_short_max_to_all
#pragma weak shmem_longdouble_max_to_all = pshmem_longdouble_max_to_all
#define shmem_longdouble_max_to_all pshmem_longdouble_max_to_all
#pragma weak shmem_float_max_to_all = pshmem_float_max_to_all
#define shmem_float_max_to_all pshmem_float_max_to_all
#pragma weak shmem_double_max_to_all = pshmem_double_max_to_all
#define shmem_double_max_to_all pshmem_double_max_to_all
#pragma weak shmem_int_min_to_all = pshmem_int_min_to_all
#define shmem_int_min_to_all pshmem_int_min_to_all
#pragma weak shmem_long_min_to_all = pshmem_long_min_to_all
#define shmem_long_min_to_all pshmem_long_min_to_all
#pragma weak shmem_longlong_min_to_all = pshmem_longlong_min_to_all
#define shmem_longlong_min_to_all pshmem_longlong_min_to_all
#pragma weak shmem_short_min_to_all = pshmem_short_min_to_all
#define shmem_short_min_to_all pshmem_short_min_to_all
#pragma weak shmem_longdouble_min_to_all = pshmem_longdouble_min_to_all
#define shmem_longdouble_min_to_all pshmem_longdouble_min_to_all
#pragma weak shmem_float_min_to_all = pshmem_float_min_to_all
#define shmem_float_min_to_all pshmem_float_min_to_all
#pragma weak shmem_double_min_to_all = pshmem_double_min_to_all
#define shmem_double_min_to_all pshmem_double_min_to_all
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * Pass in type/operation, macro builds the reduction function as
 * defined above
 *
 */

#define SHMEM_REDUCE_TYPE_OP(OpCall, Name, Type)			\
  /* @api@ */								\
  void									\
  shmem_##Name##_##OpCall##_to_all(Type *target, Type *source, int nreduce, \
				    int PE_start, int logPE_stride, int PE_size, \
				    Type *pWrk, long *pSync)		\
  {									\
    INIT_CHECK();							\
    SYMMETRY_CHECK(target, 1, "shmem_" #Name "_" #OpCall "_to_all");	\
    SYMMETRY_CHECK(source, 2, "shmem_" #Name "_" #OpCall "_to_all");	\
    __shmem_udr_##Name##_to_all(OpCall##_##Name##_func,			\
				target, source, nreduce,		\
				PE_start, logPE_stride, PE_size,	\
				pWrk, pSync);				\
  }

SHMEM_REDUCE_TYPE_OP (sum,       short,         short);
SHMEM_REDUCE_TYPE_OP (sum,       int,           int);
SHMEM_REDUCE_TYPE_OP (sum,       long,          long);
SHMEM_REDUCE_TYPE_OP (sum,       longlong,      long long);
SHMEM_REDUCE_TYPE_OP (sum,       double,        double);
SHMEM_REDUCE_TYPE_OP (sum,       float,         float);
SHMEM_REDUCE_TYPE_OP (sum,       longdouble,    long double);
SHMEM_REDUCE_TYPE_OP (sum,       complexd,      double complex);
SHMEM_REDUCE_TYPE_OP (sum,       complexf,      float complex);
SHMEM_REDUCE_TYPE_OP (prod,      short,         short);
SHMEM_REDUCE_TYPE_OP (prod,      int,           int);
SHMEM_REDUCE_TYPE_OP (prod,      long,          long);
SHMEM_REDUCE_TYPE_OP (prod,      longlong,      long long);
SHMEM_REDUCE_TYPE_OP (prod,      double,        double);
SHMEM_REDUCE_TYPE_OP (prod,      float,         float);
SHMEM_REDUCE_TYPE_OP (prod,      longdouble,    long double);
SHMEM_REDUCE_TYPE_OP (prod,      complexd,      double complex);
SHMEM_REDUCE_TYPE_OP (prod,      complexf,      float complex);
SHMEM_REDUCE_TYPE_OP (and,       short,         short);
SHMEM_REDUCE_TYPE_OP (and,       int,           int);
SHMEM_REDUCE_TYPE_OP (and,       long,          long);
SHMEM_REDUCE_TYPE_OP (and,       longlong,      long long);
SHMEM_REDUCE_TYPE_OP (or,        short,         short);
SHMEM_REDUCE_TYPE_OP (or,        int,           int);
SHMEM_REDUCE_TYPE_OP (or,        long,          long);
SHMEM_REDUCE_TYPE_OP (or,        longlong,      long long);
SHMEM_REDUCE_TYPE_OP (xor,       short,         short);
SHMEM_REDUCE_TYPE_OP (xor,       int,           int);
SHMEM_REDUCE_TYPE_OP (xor,       long,          long);
SHMEM_REDUCE_TYPE_OP (xor,       longlong,      long long);
SHMEM_REDUCE_TYPE_OP (max,       short,         short);
SHMEM_REDUCE_TYPE_OP (max,       int,           int);
SHMEM_REDUCE_TYPE_OP (max,       long,          long);
SHMEM_REDUCE_TYPE_OP (max,       longlong,      long long);
SHMEM_REDUCE_TYPE_OP (max,       double,        double);
SHMEM_REDUCE_TYPE_OP (max,       float,         float);
SHMEM_REDUCE_TYPE_OP (max,       longdouble,    long double);
SHMEM_REDUCE_TYPE_OP (min,       short,         short);
SHMEM_REDUCE_TYPE_OP (min,       int,           int);
SHMEM_REDUCE_TYPE_OP (min,       long,          long);
SHMEM_REDUCE_TYPE_OP (min,       longlong,      long long);
SHMEM_REDUCE_TYPE_OP (min,       double,        double);
SHMEM_REDUCE_TYPE_OP (min,       float,         float);
SHMEM_REDUCE_TYPE_OP (min,       longdouble,    long double);
