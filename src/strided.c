#include "shmem.h"

#include "warn.h"

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
  shmem_##Name##_iput(Type *target, const Type *source,			\
		      ptrdiff_t tst, ptrdiff_t sst, size_t nelems, int pe) \
  {									\
    size_t ti = 0, si = 0;						\
    size_t i;								\
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

_Pragma("weak shmem_iput32=shmem_int_iput")
_Pragma("weak shmem_iput64=shmem_long_iput")
_Pragma("weak shmem_iput128=shmem_longdouble_iput")

#ifdef HAVE_PSHMEM_SUPPORT
_Pragma("weak pshmem_short_iput=shmem_short_iput")
_Pragma("weak pshmem_int_iput=shmem_int_iput")
_Pragma("weak pshmem_long_iput=shmem_long_iput")
_Pragma("weak pshmem_double_iput=shmem_double_iput")
_Pragma("weak pshmem_longdouble_iput=shmem_longdouble_iput")
_Pragma("weak pshmem_longlong_iput=shmem_longlong_iput")
_Pragma("weak pshmem_iput32=shmem_iput32")
_Pragma("weak pshmem_iput64=shmem_iput64")
_Pragma("weak pshmem_iput128=shmem_iput128")
#endif /* HAVE_PSHMEM_SUPPORT */
