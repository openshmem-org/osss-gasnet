/*
 * This is the Fortran interface.  Parameter-less routines are
 * the same as C, but with the standard underscore mangling we all love.
 *
 * Also provides pass-by-reference wrappers to translate C value params
 *
 */

#include "shmem.h"

#ifdef FORTRAN_SINGLE_UNDERSCORE

#define FORTRANIFY(sym)    sym##_

#else /* ! FORTRAN_SINGLE_UNDERSCORE */

#define FORTRANIFY(sym)    sym##__

#endif /* FORTRAN_SINGLE_UNDERSCORE */

#define FORTRANIFY_VOID_VOID(F) \
  void FORTRANIFY(F) (void) { F(); }

/*
 * puts and gets
 */

#define SHMEM_FORTRAN_PUT(Name, CType)					\
  void									\
  FORTRANIFY(shmem_##Name##_put)(CType *target, const CType *src,	\
				 size_t *size, int *pe)			\
  {									\
    shmem_##CType##_put(target, src, *size, *pe);			\
  }

#define SHMEM_FORTRAN_PUT_SIZE(Size, Name, CType)			\
  void									\
  FORTRANIFY(shmem_put##Size) (CType *target, const CType *src,		\
			       size_t *size, int *pe)			\
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
FORTRANIFY(shmem_putmem)(long *target, const long *src, size_t *size, int *pe)
{
  shmem_putmem(target, src, *size, *pe);
}

#define SHMEM_FORTRAN_GET(Name, CType)					\
  void									\
  FORTRANIFY(shmem_##Name##_get)(CType *target, const CType *src,	\
				 size_t *size, int *pe)			\
  {									\
    shmem_##CType##_get(target, src, *size, *pe);			\
  }

#define SHMEM_FORTRAN_GET_SIZE(Size, Name, CType)			\
  void									\
  FORTRANIFY(shmem_get##Size) (CType *target, const CType *src,		\
			       size_t *size, int *pe)			\
  {									\
    shmem_##Name##_get(target, src, *size, *pe);			\
  }

SHMEM_FORTRAN_GET(character, int)
SHMEM_FORTRAN_GET(double,    double)
SHMEM_FORTRAN_GET(integer,   int)
SHMEM_FORTRAN_GET(logical,   long)
SHMEM_FORTRAN_GET(real,      double)

SHMEM_FORTRAN_GET_SIZE(4,    int, int)
SHMEM_FORTRAN_GET_SIZE(8,    long, long)
SHMEM_FORTRAN_GET_SIZE(32,   int, int)
SHMEM_FORTRAN_GET_SIZE(64,   long, long)
SHMEM_FORTRAN_GET_SIZE(128,  longlong, void)

void
FORTRANIFY(shmem_getmem)(long *target, const long *src, size_t *size, int *pe)
{
  shmem_getmem(target, src, *size, *pe);
}

/*
 * query functions
 */

FORTRANIFY_VOID_VOID(shmem_init)

void
FORTRANIFY(start_pes)(int *npes)
{
  start_pes(*npes);
}

#define SHMEM_FORTRAN_QUERY_PE(Name)		\
  int						\
  FORTRANIFY(Name)(void)			\
  {						\
    return Name();				\
  }

SHMEM_FORTRAN_QUERY_PE(shmem_my_pe)
SHMEM_FORTRAN_QUERY_PE(my_pe)
SHMEM_FORTRAN_QUERY_PE(_my_pe)

SHMEM_FORTRAN_QUERY_PE(shmem_num_pes)
SHMEM_FORTRAN_QUERY_PE(shmem_n_pes)
SHMEM_FORTRAN_QUERY_PE(num_pes)
SHMEM_FORTRAN_QUERY_PE(_num_pes)

char *
FORTRANIFY(shmem_hostname)(void)
{
  return shmem_hostname();
}

char *
FORTRANIFY(shmem_nodename)(void)
{
  return shmem_nodename();
}

char *
FORTRANIFY(shmem_version)(void)
{
  return shmem_version();
}

/*
 * barriers & fences
 */

void
FORTRANIFY(shmem_barrier)(int *PE_start, int *logPE_stride, int *PE_size,
			  long *pSync)
{
  shmem_barrier(*PE_start, *logPE_stride, *PE_size, pSync);
}

FORTRANIFY_VOID_VOID(shmem_barrier_all)
FORTRANIFY_VOID_VOID(shmem_fence)
FORTRANIFY_VOID_VOID(shmem_quiet)

/*
 * TODO: symmetric memory: this is subtly different in Fortran
 */

/*
 * wait operations
 */

#define FORTRANIFY_WAIT_UNTIL(Name, Type)				\
  void									\
  FORTRANIFY(shmem_##Name##_wait_until)(Type *ivar, int *cmp, Type *cmp_value) \
  {									\
    shmem_##Name##_wait_until(ivar, *cmp, *cmp_value);			\
  }

#define FORTRANIFY_WAIT(Name, Type)					\
  void									\
  FORTRANIFY(shmem_##Name##_wait)(Type *ivar, Type *cmp_value)		\
  {									\
    shmem_##Name##_wait(ivar, *cmp_value);				\
  }

FORTRANIFY_WAIT_UNTIL(short, short)
FORTRANIFY_WAIT_UNTIL(int, int)
FORTRANIFY_WAIT_UNTIL(long, long)
FORTRANIFY_WAIT_UNTIL(longlong, long long)

FORTRANIFY_WAIT(short, short)
FORTRANIFY_WAIT(int, int)
FORTRANIFY_WAIT(long, long)
FORTRANIFY_WAIT(longlong, long long)

_Pragma("weak shmem_wait_until_=shmem_long_wait_until_")
_Pragma("weak shmem_wait_=shmem_long_wait_")

/*
 * cache flushing
 */

FORTRANIFY_VOID_VOID(shmem_clear_cache_inv)
FORTRANIFY_VOID_VOID(shmem_set_cache_inv)
FORTRANIFY_VOID_VOID(shmem_udcflush)

#define FORTRANIFY_CACHE(Name)					\
  void								\
  FORTRANIFY(Name)(void *target)				\
  {								\
    Name(target);						\
  }

FORTRANIFY_CACHE(shmem_set_cache_line_inv)
FORTRANIFY_CACHE(shmem_clear_cache_line_inv)
FORTRANIFY_CACHE(shmem_udcflush_line)



/*
 * reductions
 */

/*
 * broadcasts
 */

#if 0
extern void shmem_broadcast32(void *target, const void *source, size_t nlong,
                              int PE_root, int PE_start, int logPE_stride, int PE_size,
                              long *pSync);

extern void shmem_broadcast64(void *target, const void *source, size_t nlong,
                              int PE_root, int PE_start, int logPE_stride, int PE_size,
                              long *pSync);

extern long * shmem_sync_init(void);
#endif

/*
 * fixed collects
 */

#if 0
extern void shmem_fcollect32(void *target, const void *source, size_t nlong,
                             int PE_start, int logPE_stride, int PE_size,
                             long *pSync);
extern void shmem_fcollect64(void *target, const void *source, size_t nlong,
                             int PE_start, int logPE_stride, int PE_size,
                             long *pSync);
#endif

/*
 * generalized collects
 */

#if 0
extern void shmem_collect32(void *target, const void *source, size_t nlong,
                            int PE_start, int logPE_stride, int PE_size,
                            long *pSync);
extern void shmem_collect64(void *target, const void *source, size_t nlong,
                            int PE_start, int logPE_stride, int PE_size,
                            long *pSync);
#endif
