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

#define SHMEM_FORTRAN_QUERY_PE(Name)		\
  int						\
  FORTRANIFY(Name)(void)			\
  {						\
    return Name ();				\
  }


SHMEM_FORTRAN_QUERY_PE(shmem_my_pe)
SHMEM_FORTRAN_QUERY_PE(my_pe)
SHMEM_FORTRAN_QUERY_PE(_my_pe)

SHMEM_FORTRAN_QUERY_PE(shmem_num_pes)
SHMEM_FORTRAN_QUERY_PE(num_pes)
SHMEM_FORTRAN_QUERY_PE(_num_pes)
