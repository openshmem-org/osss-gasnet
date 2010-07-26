#include "putget_nb.h"

#define SHMEM_TYPE_PUT_NB(Name, Type)					\
  __inline__ void							\
  __shmem_##Name##_put_nb(Type *target, Type *source, size_t len, int pe, \
			  shmem_handle_t *h)				\
  {									\
    *(h) = gasnet_put_nb(pe, target, source, sizeof(Type) * len);	\
  }

SHMEM_TYPE_PUT_NB(short, short)
SHMEM_TYPE_PUT_NB(int, int)
SHMEM_TYPE_PUT_NB(long, long)
SHMEM_TYPE_PUT_NB(longdouble, long double)
SHMEM_TYPE_PUT_NB(longlong, long long)
SHMEM_TYPE_PUT_NB(double, double)
SHMEM_TYPE_PUT_NB(float, float)

__inline__ void
__shmem_wait_nb(shmem_handle_t h)
{
  gasnet_wait_syncnb((gasnet_handle_t ) h);
}
