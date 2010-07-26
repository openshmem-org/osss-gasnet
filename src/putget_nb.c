#include "gasnet_safe.h"         /* call wrapper w/ err handler    */

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
