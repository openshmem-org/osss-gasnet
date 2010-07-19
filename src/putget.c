#include <stdio.h>               /* NULL                           */
#include <stdlib.h>              /* memcpy()                       */
#include <sys/types.h>           /* size_t                         */

#include "gasnet_safe.h"         /* call wrapper w/ err handler    */

#include "state.h"
#include "symmem.h"
#include "stats.h"
#include "updown.h"

/*
 * short-circuit local puts/gets, otherwise translate between
 * local/remote addresses
 * (should probably ifdef for aligned segments case)
 */

/*
 * TODO: provide address translation routine (note repeated offset code below)
 */

#define SHMEM_TYPE_PUT_EMIT(Name, Type)					\
  void									\
  shmem_##Name##_put(Type *dest, const Type *src, size_t len, int pe)	\
  {									\
    int typed_len = sizeof(Type) * len;					\
    if (__state.mype == pe) {						\
      memcpy(dest, src, typed_len);					\
    }									\
    else {								\
      size_t offset = (Type *)dest - (Type *)__symmetric_var_base(__state.mype); \
      void *rdest = (Type *)__symmetric_var_base(pe) + offset;		\
      if (! __symmetric_var_in_range(rdest, pe)) {			\
	__shmem_warn("FATAL",						\
		     "during shmem_%s_put() to PE %d, address %p not symmetric", \
		     #Name,						\
		     pe,						\
		     rdest						\
		     );							\
	__shmem_exit(1);						\
      }									\
      gasnet_put(pe, rdest, (Type *)src, typed_len);			\
    }									\
    SHMEM_STATS_PUT(pe);						\
  }

SHMEM_TYPE_PUT_EMIT(short, short)
SHMEM_TYPE_PUT_EMIT(int, int)
SHMEM_TYPE_PUT_EMIT(long, long)
SHMEM_TYPE_PUT_EMIT(longlong, long long)
SHMEM_TYPE_PUT_EMIT(longdouble, long double)
SHMEM_TYPE_PUT_EMIT(double, double)
SHMEM_TYPE_PUT_EMIT(float, float)

_Pragma("weak shmem_putmem=shmem_long_put")
_Pragma("weak shmem_put=shmem_long_put")
_Pragma("weak shmem_put32=shmem_int_put")
_Pragma("weak shmem_put64=shmem_long_put")
_Pragma("weak shmem_put128=shmem_longdouble_put")

  
#define SHMEM_TYPE_GET_EMIT(Name, Type)					\
  void									\
  shmem_##Name##_get(Type *dest, const Type *src, size_t len, int pe) \
  {									\
    int typed_len = sizeof(Type) * len;					\
    if (__state.mype == pe) {						\
      memcpy(dest, src, typed_len);					\
    }									\
    else {								\
      size_t offset = (Type *)src - (Type *)__symmetric_var_base(__state.mype); \
      void *their_src = (Type *)__symmetric_var_base(pe) + offset;	\
      if (! __symmetric_var_in_range(their_src, pe)) {			\
	__shmem_warn("FATAL",						\
		     "during shmem_%s_get() from PE %d, address %p not symmetric", \
		     #Name,						\
		     pe,						\
		     their_src						\
		     );							\
	__shmem_exit(1);						\
      }									\
      gasnet_get(dest, pe, their_src, typed_len);			\
    }									\
    SHMEM_STATS_GET(pe);						\
  }

SHMEM_TYPE_GET_EMIT(short, short)
SHMEM_TYPE_GET_EMIT(int, int)
SHMEM_TYPE_GET_EMIT(long, long)
SHMEM_TYPE_GET_EMIT(longdouble, long double)
SHMEM_TYPE_GET_EMIT(longlong, long long)
SHMEM_TYPE_GET_EMIT(double, double)
SHMEM_TYPE_GET_EMIT(float, float)

_Pragma("weak shmem_getmem=shmem_long_get")
_Pragma("weak shmem_get=shmem_long_get")
_Pragma("weak shmem_get32=shmem_int_get")
_Pragma("weak shmem_get64=shmem_long_get")
_Pragma("weak shmem_get128=shmem_longdouble_get")

#define SHMEM_TYPE_PUT_NBI(Name, Type)					\
  void									\
  __shmem_##Name##_put_nbi(Type *target, Type *source, size_t len, int pe) \
  {									\
    gasnet_put_nbi(pe, target, source, sizeof(Type) * len);		\
  }

SHMEM_TYPE_PUT_NBI(short, short)
SHMEM_TYPE_PUT_NBI(int, int)
SHMEM_TYPE_PUT_NBI(long, long)
SHMEM_TYPE_PUT_NBI(longdouble, long double)
SHMEM_TYPE_PUT_NBI(longlong, long long)
SHMEM_TYPE_PUT_NBI(double, double)
SHMEM_TYPE_PUT_NBI(float, float)

void
__shmem_wait_syncnbi_puts(void)
{
  gasnet_wait_syncnbi_puts();
}


#define SHMEM_TYPE_P(Name, Type)					\
  void									\
  shmem_##Name##_p(Type *dest, Type value, int pe)			\
  {									\
    if (__state.mype == pe) {						\
      *dest = value;							\
    }									\
    else {								\
      int typed_len = sizeof(Type);					\
      size_t offset = (Type *)dest - (Type *)__symmetric_var_base(__state.mype); \
      void *rdest = (Type *)__symmetric_var_base(pe) + offset;		\
      if (! __symmetric_var_in_range(rdest, pe)) {			\
	__shmem_warn("FATAL",						\
		     "during shmem_%s_p() to PE %d, address %p not symmetric", \
		     #Name,						\
		     pe,						\
		     rdest						\
		     );							\
	__shmem_exit(1);						\
      }									\
      gasnet_put_val(pe, rdest, value, typed_len);			\
    }									\
    SHMEM_STATS_PUT(pe);						\
  }

SHMEM_TYPE_P(char, char)
SHMEM_TYPE_P(short, short)
SHMEM_TYPE_P(int, int)
SHMEM_TYPE_P(long, long)
SHMEM_TYPE_P(longlong, long long)
SHMEM_TYPE_P(float, float)
SHMEM_TYPE_P(double, double)
SHMEM_TYPE_P(longdouble, long double)

  
#define SHMEM_TYPE_G(Name, Type)					\
  Type									\
  shmem_##Name##_g(Type *src, int pe)					\
  {									\
    Type retval;							\
    if (__state.mype == pe) {						\
      retval = *src;							\
    }									\
    else {								\
      int typed_len = sizeof(Type);					\
      size_t offset = (Type *)src - (Type *)__symmetric_var_base(__state.mype); \
      void *their_src = (Type *)__symmetric_var_base(pe) + offset;	\
      if (! __symmetric_var_in_range(their_src, pe)) {			\
	__shmem_warn("FATAL",						\
		     "during shmem_%s_g() from PE %d, address %p not symmetric", \
		     #Name,						\
		     pe,						\
		     their_src						\
		     );							\
	__shmem_exit(1);						\
      }									\
      retval = gasnet_get_val(pe, their_src, typed_len);		\
    }									\
    SHMEM_STATS_GET(pe);						\
    return retval;							\
  }


SHMEM_TYPE_G(char, char)
SHMEM_TYPE_G(short, short)
SHMEM_TYPE_G(int, int)
SHMEM_TYPE_G(long, long)
SHMEM_TYPE_G(longlong, long long)
SHMEM_TYPE_G(float, float)
SHMEM_TYPE_G(double, double)
SHMEM_TYPE_G(longdouble, long double)
