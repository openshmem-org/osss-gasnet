/* TODO: the whole thing !!! */

#include "shmem.h"

#include "warn.h"

#define SHMEM_EMIT_IPUT(Name, Type)					\
  void									\
  shmem_##Name##_iput(Type *target, const Type *source,			\
		      ptrdiff_t tst, ptrdiff_t sst, size_t len, int pe)	\
  {									\
    __warn(SHMEM_LOG_FATAL,						\
	   "shmem_%s_iput not yet implemented",				\
	   #Name							\
	   );								\
  }

SHMEM_EMIT_IPUT(short, short)
SHMEM_EMIT_IPUT(int, int)
SHMEM_EMIT_IPUT(long, long)
SHMEM_EMIT_IPUT(float, float)
SHMEM_EMIT_IPUT(longdouble, long double)
SHMEM_EMIT_IPUT(longlong, long long)


_Pragma("weak shmem_iput32=shmem_int_iput")
_Pragma("weak shmem_iput64=shmem_long_iput")
_Pragma("weak shmem_iput128=shmem_longdouble_iput")
