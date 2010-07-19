/* TODO: would help to have some code that does something :-) */

#include "reduce-common.h"

#define SHMEM_TYPE_MAX_TO_ALL(Name, Type)                               \
  void									\
  shmem_##Name##_max_to_all(Type *target, Type *source, int nreduce,	\
			    int PE_start, int logPE_stride, int PE_size, \
			    Type *pWrk, long *pSync)			\
  {									\
  }

SHMEM_TYPE_MAX_TO_ALL(int, int)
SHMEM_TYPE_MAX_TO_ALL(long, long)
SHMEM_TYPE_MAX_TO_ALL(longlong, long long)
SHMEM_TYPE_MAX_TO_ALL(short, short)
SHMEM_TYPE_MAX_TO_ALL(longdouble, long double)
SHMEM_TYPE_MAX_TO_ALL(float, float)
SHMEM_TYPE_MAX_TO_ALL(double, double)
