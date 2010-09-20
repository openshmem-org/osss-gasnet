#include "shmem.h"

#define FORTRANIFY(sym)    sym##_

void
FORTRANIFY(shmem_init)(void)
{
  shmem_init();
}

void
FORTRANIFY(start_pes)(int *npes)
{
  start_pes(*npes);
}

#define SHMEM_FORTRAN_PUT(Name, CType)					\
   void							\
  FORTRANIFY(shmem_##Name##_put)(CType *target, const CType *src, size_t *size, int *pe) \
  {									\
    shmem_##CType##_put(target, src, *size, *pe);			\
  }

#define SHMEM_FORTRAN_PUT_SIZE(Size, Name, CType)			\
   void							\
  FORTRANIFY(shmem_put##Size) (CType *target, const CType *src, size_t *size, int *pe) \
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

void
FORTRANIFY(shmem_put)(long *target, const long *src, size_t *size, int *pe)
{
  shmem_put(target, src, *size, *pe);
}

int
FORTRANIFY(shmem_my_pe)(void)
{
  return shmem_my_pe();
}

int
FORTRANIFY(my_pe)(void)
{
  return my_pe();
}

int
FORTRANIFY(_my_pe)(void)
{
  return _my_pe();
}

int
FORTRANIFY(shmem_num_pes)(void)
{
  return shmem_num_pes();
}

int
FORTRANIFY(num_pes)(void)
{
  return num_pes();
}

int
FORTRANIFY(_num_pes)(void)
{
  return _num_pes();
}
