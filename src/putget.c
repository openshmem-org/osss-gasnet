/* (c) 2011 University of Houston.  All rights reserved. */


#include <stdio.h>               /* NULL                           */
#include <string.h>              /* memcpy()                       */
#include <sys/types.h>           /* size_t                         */

#include "state.h"
#include "symmem.h"
#include "comms.h"
#include "globalvar.h"
#include "updown.h"
#include "trace.h"
#include "atomic.h"
#include "utils.h"
#include "symmtest.h"

#include "mpp/pshmem.h"

/*
 * short-circuit local puts/gets, otherwise translate between
 * local/remote addresses
 * (should probably ifdef for aligned segments case)
 */

#define SHMEM_TYPE_PUT(Name, Type)					\
  /* @api@ */								\
  void									\
  pshmem_##Name##_put(Type *dest, const Type *src, size_t len, int pe)	\
  {									\
    int typed_len = sizeof(Type) * len;					\
    INIT_CHECK();							\
    PE_RANGE_CHECK(pe);							\
    SYMMETRY_CHECK(dest, 1, "shmem_" #Name "_put");			\
    if (GET_STATE(mype) == pe) {					\
      memmove(dest, src, typed_len);					\
      LOAD_STORE_FENCE();						\
    }									\
    else {								\
      void *rdest = __shmem_symmetric_addr_lookup(dest, pe);		\
      __shmem_comms_put(rdest, (Type *) src, typed_len, pe);		\
    }									\
  }

SHMEM_TYPE_PUT(char, char)
SHMEM_TYPE_PUT(short, short)
SHMEM_TYPE_PUT(int, int)
SHMEM_TYPE_PUT(long, long)
SHMEM_TYPE_PUT(longlong, long long)
SHMEM_TYPE_PUT(longdouble, long double)
SHMEM_TYPE_PUT(double, double)
SHMEM_TYPE_PUT(float, float)
SHMEM_TYPE_PUT(complexf, COMPLEXIFY(float))
SHMEM_TYPE_PUT(complexd, COMPLEXIFY(double))

#pragma weak pshmem_putmem = pshmem_char_put
#pragma weak pshmem_put32 = pshmem_int_put
#pragma weak pshmem_put64 = pshmem_long_put
#pragma weak pshmem_put128 = pshmem_longdouble_put

#pragma weak shmem_short_put = pshmem_short_put
#pragma weak shmem_int_put = pshmem_int_put
#pragma weak shmem_long_put = pshmem_long_put
#pragma weak shmem_longdouble_put = pshmem_longdouble_put
#pragma weak shmem_longlong_put = pshmem_longlong_put
#pragma weak shmem_double_put = pshmem_double_put
#pragma weak shmem_float_put = pshmem_float_put
#pragma weak shmem_complexf_put = pshmem_complexf_put
#pragma weak shmem_complexd_put = pshmem_complexd_put
#pragma weak shmem_putmem = pshmem_putmem
#pragma weak shmem_put32 = pshmem_put32
#pragma weak shmem_put64 = pshmem_put64
#pragma weak shmem_put128 = pshmem_put128

/* #pragma weak pshmem_put = pshmem_long_put */
/* #pragma weak shmem_put = pshmem_put */

#define SHMEM_TYPE_GET(Name, Type)					\
  /* @api@ */								\
  void									\
  pshmem_##Name##_get(Type *dest, const Type *src, size_t len, int pe)	\
  {									\
    int typed_len = sizeof(Type) * len;					\
    INIT_CHECK();							\
    PE_RANGE_CHECK(pe);							\
    SYMMETRY_CHECK(src, 2, "shmem_" #Name "_get");			\
    if (GET_STATE(mype) == pe) {					\
      memmove(dest, src, typed_len);					\
      LOAD_STORE_FENCE();						\
    }									\
    else {								\
      void *their_src = __shmem_symmetric_addr_lookup((void *) src, pe); \
      __shmem_comms_get(dest, their_src, typed_len, pe);		\
    }									\
  }

SHMEM_TYPE_GET(char, char)
SHMEM_TYPE_GET(short, short)
SHMEM_TYPE_GET(int, int)
SHMEM_TYPE_GET(long, long)
SHMEM_TYPE_GET(longdouble, long double)
SHMEM_TYPE_GET(longlong, long long)
SHMEM_TYPE_GET(double, double)
SHMEM_TYPE_GET(float, float)
SHMEM_TYPE_GET(complexf, COMPLEXIFY(float))
SHMEM_TYPE_GET(complexd, COMPLEXIFY(double))

#pragma weak pshmem_getmem = pshmem_char_get
#pragma weak pshmem_get32 = pshmem_int_get
#pragma weak pshmem_get64 = pshmem_long_get
#pragma weak pshmem_get128 = pshmem_longdouble_get

#pragma weak shmem_short_get = pshmem_short_get
#pragma weak shmem_int_get = pshmem_int_get
#pragma weak shmem_long_get = pshmem_long_get
#pragma weak shmem_longdouble_get = pshmem_longdouble_get
#pragma weak shmem_longlong_get = pshmem_longlong_get
#pragma weak shmem_double_get = pshmem_double_get
#pragma weak shmem_float_get = pshmem_float_get
#pragma weak shmem_complexf_get = pshmem_complexf_get
#pragma weak shmem_complexd_get = pshmem_complexd_get
#pragma weak shmem_getmem = pshmem_getmem
#pragma weak shmem_get32 = pshmem_get32
#pragma weak shmem_get64 = pshmem_get64
#pragma weak shmem_get128 = pshmem_get128

/* #pragma weak pshmem_get = pshmem_long_get */
/* #pragma weak shmem_get = pshmem_get */


/*
 * gasnet_(get|get)_val can't handle bigger types..
 */

#define SHMEM_TYPE_P_WRAPPER(Name, Type)				\
  /* @api@ */								\
  void									\
  pshmem_##Name##_p(Type *dest, Type value, int pe)			\
  {									\
    pshmem_##Name##_put(dest, &value, 1, pe);				\
  }

SHMEM_TYPE_P_WRAPPER(float, float)
SHMEM_TYPE_P_WRAPPER(double, double)
SHMEM_TYPE_P_WRAPPER(longdouble, long double)
SHMEM_TYPE_P_WRAPPER(longlong, long long)
SHMEM_TYPE_P_WRAPPER(short, short)
SHMEM_TYPE_P_WRAPPER(int, int)
SHMEM_TYPE_P_WRAPPER(long, long)

#pragma weak shmem_short_p = pshmem_short_p
#pragma weak shmem_int_p = pshmem_int_p
#pragma weak shmem_long_p = pshmem_long_p
#pragma weak shmem_longdouble_p = pshmem_longdouble_p
#pragma weak shmem_longlong_p = pshmem_longlong_p
#pragma weak shmem_double_p = pshmem_double_p
#pragma weak shmem_float_p = pshmem_float_p

#define SHMEM_TYPE_G_WRAPPER(Name, Type)				\
  /* @api@ */								\
  Type									\
  pshmem_##Name##_g(Type *addr, int pe)					\
  {									\
    Type retval;							\
    pshmem_##Name##_get(&retval, addr, 1, pe);				\
    return retval;							\
  }

SHMEM_TYPE_G_WRAPPER(float, float)
SHMEM_TYPE_G_WRAPPER(double, double)
SHMEM_TYPE_G_WRAPPER(longlong, long long)
SHMEM_TYPE_G_WRAPPER(longdouble, long double)
SHMEM_TYPE_G_WRAPPER(short, short)
SHMEM_TYPE_G_WRAPPER(int, int)
SHMEM_TYPE_G_WRAPPER(long, long)

#pragma weak shmem_short_g = pshmem_short_g
#pragma weak shmem_int_g = pshmem_int_g
#pragma weak shmem_long_g = pshmem_long_g
#pragma weak shmem_longdouble_g = pshmem_longdouble_g
#pragma weak shmem_longlong_g = pshmem_longlong_g
#pragma weak shmem_double_g = pshmem_double_g
#pragma weak shmem_float_g = pshmem_float_g
