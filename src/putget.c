#include <stdio.h>               /* NULL                           */
#include <string.h>              /* memcpy()                       */
#include <sys/types.h>           /* size_t                         */

#include "state.h"
#include "symmem.h"
#include "comms.h"
#include "updown.h"
#include "trace.h"

void
symmetric_test_with_abort(void *remote_addr,
			  void *local_addr,
			  const char *name,
			  const char *routine)
{
  if (remote_addr == NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "shmem_%s_%s: address at %p is not symmetric",
		  name, routine,
		  local_addr
		  );
    /* NOT REACHED */
  }
}

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
    if (__state.mype == pe) {						\
      memcpy(dest, src, typed_len);					\
    }									\
    else {								\
      void *rdest = __symmetric_addr_lookup(dest, pe);			\
      symmetric_test_with_abort((void *) rdest, (void *) dest, #Name, "put");	\
      __comms_put(rdest, (Type *) src, typed_len, pe);			\
    }									\
  }

SHMEM_TYPE_PUT(short, short)
SHMEM_TYPE_PUT(int, int)
SHMEM_TYPE_PUT(long, long)
SHMEM_TYPE_PUT(longlong, long long)
SHMEM_TYPE_PUT(longdouble, long double)
SHMEM_TYPE_PUT(double, double)
SHMEM_TYPE_PUT(float, float)

#pragma weak pshmem_putmem = pshmem_long_put
#pragma weak pshmem_put = pshmem_long_put
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
#pragma weak shmem_putmem = pshmem_putmem
#pragma weak shmem_put = pshmem_put
#pragma weak shmem_put32 = pshmem_put32
#pragma weak shmem_put64 = pshmem_put64
#pragma weak shmem_put128 = pshmem_put128

#define SHMEM_TYPE_GET(Name, Type)					\
  /* @api@ */								\
  void									\
  pshmem_##Name##_get(Type *dest, const Type *src, size_t len, int pe)	\
  {									\
    int typed_len = sizeof(Type) * len;					\
    if (__state.mype == pe) {						\
      memcpy(dest, src, typed_len);					\
    }									\
    else {								\
      void *their_src = __symmetric_addr_lookup((Type *) src, pe);	\
      symmetric_test_with_abort((void *) their_src, (void *) src, #Name, "get"); \
      __comms_get(dest, their_src, typed_len, pe);			\
    }									\
  }

SHMEM_TYPE_GET(short, short)
SHMEM_TYPE_GET(int, int)
SHMEM_TYPE_GET(long, long)
SHMEM_TYPE_GET(longdouble, long double)
SHMEM_TYPE_GET(longlong, long long)
SHMEM_TYPE_GET(double, double)
SHMEM_TYPE_GET(float, float)

#pragma weak pshmem_getmem = pshmem_long_get
#pragma weak pshmem_get = pshmem_long_get
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
#pragma weak shmem_getmem = pshmem_getmem
#pragma weak shmem_get = pshmem_get
#pragma weak shmem_get32 = pshmem_get32
#pragma weak shmem_get64 = pshmem_get64
#pragma weak shmem_get128 = pshmem_get128


/*
 * gasnet_(get|get)_val can't handle bigger types..
 */

#define SHMEM_TYPE_P_WRAPPER(Name, Type)				\
  /* @api@ */								\
  void									\
  pshmem_##Name##_p(Type *dest, Type value, int pe)			\
  {									\
    pshmem_##Name##_put(dest, &value, sizeof(value), pe);		\
  }

SHMEM_TYPE_P_WRAPPER(float, float)
SHMEM_TYPE_P_WRAPPER(double, double)
SHMEM_TYPE_P_WRAPPER(longdouble, long double)
SHMEM_TYPE_P_WRAPPER(longlong, long long)

#define SHMEM_TYPE_P(Name, Type)					\
  /* @api@ */								\
  void									\
  pshmem_##Name##_p(Type *dest, Type value, int pe)			\
  {									\
    if (__state.mype == pe) {						\
      *dest = value;							\
    }									\
    else {								\
      void *rdest = __symmetric_addr_lookup(dest, pe);			\
      symmetric_test_with_abort((void *) rdest, (void *) dest, #Name, "p");	\
      __comms_put_val(rdest, value, sizeof(Type), pe);			\
    }									\
  }

SHMEM_TYPE_P(short, short)
SHMEM_TYPE_P(int, int)
SHMEM_TYPE_P(long, long)

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
  pshmem_##Name##_g(Type *dest, int pe)					\
  {									\
    Type retval;							\
    pshmem_##Name##_get(dest, &retval, sizeof(retval), pe);		\
    return retval;							\
  }

SHMEM_TYPE_G_WRAPPER(float, float)
SHMEM_TYPE_G_WRAPPER(double, double)
SHMEM_TYPE_G_WRAPPER(longlong, long long)
SHMEM_TYPE_G_WRAPPER(longdouble, long double)

#define SHMEM_TYPE_G(Name, Type)					\
  /* @api@ */								\
  Type									\
  pshmem_##Name##_g(Type *src, int pe)					\
  {									\
    Type retval;							\
    if (__state.mype == pe) {						\
      retval = *src;							\
    }									\
    else {								\
      void *their_src = __symmetric_addr_lookup(src, pe);		\
      symmetric_test_with_abort((void *) their_src, (void *) src, #Name, "p"); \
      retval = (Type) __comms_get_val(their_src, sizeof(retval), pe);	\
    }									\
    return retval;							\
  }

SHMEM_TYPE_G(short, short)
SHMEM_TYPE_G(int, int)
SHMEM_TYPE_G(long, long)

#pragma weak shmem_short_g = pshmem_short_g
#pragma weak shmem_int_g = pshmem_int_g
#pragma weak shmem_long_g = pshmem_long_g
#pragma weak shmem_longdouble_g = pshmem_longdouble_g
#pragma weak shmem_longlong_g = pshmem_longlong_g
#pragma weak shmem_double_g = pshmem_double_g
#pragma weak shmem_float_g = pshmem_float_g

/*
 * non-blocking extensions
 */

#define SHMEM_TYPE_PUT_NB(Name, Type)					\
  /* @api@ */								\
  void *								\
  pshmem_##Name##_put_nb(Type *target, Type *source, size_t len, int pe)	\
  {									\
    return __comms_##Name##_put_nb(target, source, len, pe);		\
  }

SHMEM_TYPE_PUT_NB(short, short)
SHMEM_TYPE_PUT_NB(int, int)
SHMEM_TYPE_PUT_NB(long, long)
SHMEM_TYPE_PUT_NB(longdouble, long double)
SHMEM_TYPE_PUT_NB(longlong, long long)
SHMEM_TYPE_PUT_NB(double, double)
SHMEM_TYPE_PUT_NB(float, float)

#pragma weak shmem_short_put_nb = pshmem_short_put_nb
#pragma weak shmem_int_put_nb = pshmem_int_put_nb
#pragma weak shmem_long_put_nb = pshmem_long_put_nb
#pragma weak shmem_longdouble_put_nb = pshmem_longdouble_put_nb
#pragma weak shmem_longlong_put_nb = pshmem_longlong_put_nb
#pragma weak shmem_double_g = pshmem_double_put_nb
#pragma weak shmem_float_put_nb = pshmem_float_put_nb
