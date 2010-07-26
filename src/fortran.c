#include "shmem.h"

void
shmem_init_(void)
{
  shmem_init();
}

void
start_pes_(int *npes)
{
  shmem_init();
}

#define SHMEM_FORTRAN_PUT(Name, CType)					\
  __inline__ void							\
  shmem_##Name##_put_(CType *target, const CType *src, size_t *size, int *pe) \
  {									\
    shmem_##CType##_put(target, src, *size, *pe);			\
  }

#define SHMEM_FORTRAN_PUT_SIZE(Size, Name, CType)			\
  __inline__ void							\
  shmem_put##Size##_ (CType *target, const CType *src, size_t *size, int *pe) \
  {									\
    shmem_##Name##_put(target, src, *size, *pe);			\
  }

SHMEM_FORTRAN_PUT(character, int)
SHMEM_FORTRAN_PUT(double,    double)
SHMEM_FORTRAN_PUT(integer,   int)
SHMEM_FORTRAN_PUT(logical,   long)
SHMEM_FORTRAN_PUT(real,      double)

SHMEM_FORTRAN_PUT_SIZE(4,    int, int)
SHMEM_FORTRAN_PUT_SIZE(8,    long, long)
SHMEM_FORTRAN_PUT_SIZE(32,   int, int)
SHMEM_FORTRAN_PUT_SIZE(64,   long, long)
SHMEM_FORTRAN_PUT_SIZE(128,  longlong, void)

__inline__ void
shmem_put_(long *target, const long *src, size_t *size, int *pe)
{
  shmem_put(target, src, *size, *pe);
}
