/*
 * This is the Fortran interface.  Parameter-less routines are
 * the same as C, but with the standard underscore mangling we all love.
 *
 * Also provides pass-by-reference wrappers to translate C value params
 *
 */

#include "pshmem.h"

#include "fortran-common.h"

/*
 * puts and gets
 */

#define SHMEM_FORTRAN_PUT(FName, CName, CType)				\
  void									\
  FORTRANIFY(pshmem_##FName##_put)(CType *target, const CType *src,	\
				  size_t *size, int *pe)		\
  {									\
    pshmem_##CName##_put(target, src, *size, *pe);			\
  }

#define SHMEM_FORTRAN_PUT_SIZE(Size, CName, CType)		\
  void								\
  FORTRANIFY(pshmem_put##Size) (CType *target, const CType *src,	\
			       size_t *size, int *pe)		\
  {								\
    pshmem_##CName##_put(target, src, *size, *pe);		\
  }

SHMEM_FORTRAN_PUT(character, int, int)
SHMEM_FORTRAN_PUT(double,    double, double)
SHMEM_FORTRAN_PUT(integer,   long, long)
SHMEM_FORTRAN_PUT(logical,   long, long)
SHMEM_FORTRAN_PUT(real,      double, double)

SHMEM_FORTRAN_PUT(complex,   complexd, COMPLEXIFY(double))

SHMEM_FORTRAN_PUT_SIZE(4,    int, int)
SHMEM_FORTRAN_PUT_SIZE(8,    long, long)
SHMEM_FORTRAN_PUT_SIZE(32,   int, int)
SHMEM_FORTRAN_PUT_SIZE(64,   long, long)
SHMEM_FORTRAN_PUT_SIZE(128,  longlong, void)

void
FORTRANIFY(pshmem_putmem)(long *target, const long *src, size_t *size, int *pe)
{
  pshmem_putmem(target, src, *size, *pe);
}

#pragma weak shmem_character_put_ = pshmem_character_put_
#pragma weak shmem_double_put_ = pshmem_double_put_
#pragma weak shmem_integer_put_ = pshmem_integer_put_
#pragma weak shmem_logical_put_ = pshmem_logical_put_
#pragma weak shmem_real_put_ = pshmem_real_put_

#pragma weak shmem_put4_ = pshmem_put4_
#pragma weak shmem_put8_ = pshmem_put8_
#pragma weak shmem_put32_ = pshmem_put32_
#pragma weak shmem_put64_ = pshmem_put64_
#pragma weak shmem_put128_ = pshmem_put128_

#pragma weak shmem_putmem_ = pshmem_putmem_

#define SHMEM_FORTRAN_GET(Name, CType)					\
  void									\
  FORTRANIFY(pshmem_##Name##_get)(CType *target, const CType *src,	\
				 size_t *size, int *pe)			\
  {									\
    pshmem_##CType##_get(target, src, *size, *pe);			\
  }

#define SHMEM_FORTRAN_GET_SIZE(Size, Name, CType)		\
  void								\
  FORTRANIFY(pshmem_get##Size) (CType *target, const CType *src,	\
			       size_t *size, int *pe)		\
  {								\
    pshmem_##Name##_get(target, src, *size, *pe);		\
  }

SHMEM_FORTRAN_GET(character, int)
SHMEM_FORTRAN_GET(double,    double)
SHMEM_FORTRAN_GET(integer,   long)
SHMEM_FORTRAN_GET(logical,   long)
SHMEM_FORTRAN_GET(real,      double)

SHMEM_FORTRAN_GET_SIZE(4,    int, int)
SHMEM_FORTRAN_GET_SIZE(8,    long, long)
SHMEM_FORTRAN_GET_SIZE(32,   int, int)
SHMEM_FORTRAN_GET_SIZE(64,   long, long)
SHMEM_FORTRAN_GET_SIZE(128,  longdouble, long double)

void
FORTRANIFY(pshmem_getmem)(long *target, const long *src, size_t *size, int *pe)
{
  pshmem_getmem(target, src, *size, *pe);
}

#pragma weak shmem_character_get_ = pshmem_character_get_
#pragma weak shmem_double_get_ = pshmem_double_get_
#pragma weak shmem_integer_get_ = pshmem_integer_get_
#pragma weak shmem_logical_get_ = pshmem_logical_get_
#pragma weak shmem_real_get_ = pshmem_real_get_

#pragma weak shmem_get4_ = pshmem_get4_
#pragma weak shmem_get8_ = pshmem_get8_
#pragma weak shmem_get32_ = pshmem_get32_
#pragma weak shmem_get64_ = pshmem_get64_
#pragma weak shmem_get128_ = pshmem_get128_

#pragma weak shmem_getmem_ = pshmem_getmem_

/*
 * strided puts and gets
 *
 */

#define SHMEM_FORTRAN_IPUT(Name, CType)					\
  void									\
  FORTRANIFY(pshmem_##Name##_iput)(CType *target, const CType *src,	\
				   int *tst, int *sst,			\
				   int *size, int *pe)			\
  {									\
    pshmem_##CType##_iput(target, src, *tst, *sst, *size, *pe);	\
  }

#include <stdio.h>

#define SHMEM_FORTRAN_IPUT_SIZE(Size, Name, CType)			\
  void									\
  FORTRANIFY(pshmem_iput##Size) (CType *target, const CType *src,	\
				 int *tst, int *sst,			\
				 int *size, int *pe)			\
  {									\
    pshmem_##Name##_iput(target, src, *tst, *sst, *size, *pe);		\
  }

SHMEM_FORTRAN_IPUT(double,    double)
SHMEM_FORTRAN_IPUT(integer,   long)
SHMEM_FORTRAN_IPUT(logical,   long)
SHMEM_FORTRAN_IPUT(real,      double)

SHMEM_FORTRAN_IPUT_SIZE(4,    int, int)
SHMEM_FORTRAN_IPUT_SIZE(8,    long, long)
SHMEM_FORTRAN_IPUT_SIZE(32,   int, int)
SHMEM_FORTRAN_IPUT_SIZE(64,   long, long)
SHMEM_FORTRAN_IPUT_SIZE(128,  longdouble, long double)

#pragma weak shmem_double_iput_ = pshmem_double_iput_
#pragma weak shmem_integer_iput_ = pshmem_integer_iput_
#pragma weak shmem_logical_iput_ = pshmem_logical_iput_
#pragma weak shmem_real_iput_ = pshmem_real_iput_

#pragma weak shmem_iput4_ = pshmem_iput4_
#pragma weak shmem_iput8_ = pshmem_iput8_
#pragma weak shmem_iput32_ = pshmem_iput32_
#pragma weak shmem_iput64_ = pshmem_iput64_
#pragma weak shmem_iput128_ = pshmem_iput128_

#define SHMEM_FORTRAN_IGET(Name, CType)					\
  void									\
  FORTRANIFY(pshmem_##Name##_iget)(CType *target, const CType *src,	\
				   int *tst, int *sst,			\
				   int *size, int *pe)			\
  {									\
    pshmem_##CType##_iget(target, src, *tst, *sst, *size, *pe);		\
  }

#define SHMEM_FORTRAN_IGET_SIZE(Size, Name, CType)			\
  void									\
  FORTRANIFY(pshmem_iget##Size) (CType *target, const CType *src,	\
				 int *tst, int *sst,			\
				 int *size, int *pe)			\
  {									\
    pshmem_##Name##_iget(target, src, *tst, *sst, *size, *pe);		\
  }

SHMEM_FORTRAN_IGET(double,    double)
SHMEM_FORTRAN_IGET(integer,   long)
SHMEM_FORTRAN_IGET(logical,   long)
SHMEM_FORTRAN_IGET(real,      double)

SHMEM_FORTRAN_IGET_SIZE(4,    int, int)
SHMEM_FORTRAN_IGET_SIZE(8,    long, long)
SHMEM_FORTRAN_IGET_SIZE(32,   int, int)
SHMEM_FORTRAN_IGET_SIZE(64,   long, long)
SHMEM_FORTRAN_IGET_SIZE(128,  longdouble, long double)

#pragma weak shmem_double_iget_ = pshmem_double_iget_
#pragma weak shmem_integer_iget_ = pshmem_integer_iget_
#pragma weak shmem_logical_iget_ = pshmem_logical_iget_
#pragma weak shmem_real_iget_ = pshmem_real_iget_

#pragma weak shmem_iget4_ = pshmem_iget4_
#pragma weak shmem_iget8_ = pshmem_iget8_
#pragma weak shmem_iget32_ = pshmem_iget32_
#pragma weak shmem_iget64_ = pshmem_iget64_
#pragma weak shmem_iget128_ = pshmem_iget128_

/*
 * init & query functions
 */

void
FORTRANIFY(pstart_pes)(int *npes)
{
  pstart_pes(*npes);
}

FORTRANIFY_VOID_VOID(pshmem_init)
FORTRANIFY_VOID_VOID(pshmem_finalize)

#define SHMEM_FORTRAN_QUERY_PE(Name)		\
  int						\
  FORTRANIFY(Name)(void)			\
  {						\
    return Name();				\
  }

SHMEM_FORTRAN_QUERY_PE(pshmem_my_pe)
SHMEM_FORTRAN_QUERY_PE(pmy_pe)
SHMEM_FORTRAN_QUERY_PE(p_my_pe)

SHMEM_FORTRAN_QUERY_PE(pshmem_num_pes)
SHMEM_FORTRAN_QUERY_PE(pshmem_n_pes)
SHMEM_FORTRAN_QUERY_PE(pnum_pes)
SHMEM_FORTRAN_QUERY_PE(p_num_pes)

char *
FORTRANIFY(pshmem_nodename)(void)
{
  return pshmem_nodename();
}

int
FORTRANIFY(pshmem_version)(int *major, int *minor)
{
  return pshmem_version(major, minor);
}

#pragma weak start_pes_ = pstart_pes_
#pragma weak shmem_init_ = pshmem_init_
#pragma weak shmem_finalize_ = pshmem_finalize_
#pragma weak shmem_nodename_ = pshmem_nodename_
#pragma weak shmem_version_ = pshmem_version_

#pragma weak shmem_my_pe_ = pshmem_my_pe_
#pragma weak my_pe_ = pmy_pe_
#pragma weak _my_pe_ = p_my_pe_

#pragma weak shmem_num_pes_ = pshmem_num_pes_
#pragma weak shmem_n_pes_ = pshmem_n_pes_
#pragma weak num_pes_ = pnum_pes_
#pragma weak _num_pes_ = p_num_pes_

/*
 * barriers & fences
 */

void
FORTRANIFY(pshmem_barrier)(int *PE_start, int *logPE_stride, int *PE_size,
			  long *pSync)
{
  pshmem_barrier(*PE_start, *logPE_stride, *PE_size, pSync);
}

FORTRANIFY_VOID_VOID(pshmem_barrier_all)
FORTRANIFY_VOID_VOID(pshmem_fence)
FORTRANIFY_VOID_VOID(pshmem_quiet)

#pragma weak shmem_barrier_ = pshmem_barrier_
#pragma weak shmem_barrier_all_ = pshmem_barrier_all_
#pragma weak shmem_fence_ = pshmem_fence_
#pragma weak shmem_quiet_ = pshmem_quiet_


/*
 * wait operations
 */

#define FORTRANIFY_WAIT_UNTIL(Name, Type)				\
  void									\
  FORTRANIFY(pshmem_##Name##_wait_until)(Type *ivar, int *cmp, Type *cmp_value) \
  {									\
    pshmem_##Name##_wait_until(ivar, *cmp, *cmp_value);			\
  }

#define FORTRANIFY_WAIT(Name, Type)				\
  void								\
  FORTRANIFY(pshmem_##Name##_wait)(Type *ivar, Type *cmp_value)	\
  {								\
    pshmem_##Name##_wait(ivar, *cmp_value);			\
  }

FORTRANIFY_WAIT_UNTIL(short, short)
FORTRANIFY_WAIT_UNTIL(int, int)
FORTRANIFY_WAIT_UNTIL(long, long)
FORTRANIFY_WAIT_UNTIL(longlong, long long)

FORTRANIFY_WAIT(short, short)
FORTRANIFY_WAIT(int, int)
FORTRANIFY_WAIT(long, long)
FORTRANIFY_WAIT(longlong, long long)

#pragma weak pshmem_wait_until_ = pshmem_long_wait_until_
#pragma weak pshmem_wait_ = pshmem_long_wait_

#pragma weak shmem_short_wait_until_ = pshmem_short_wait_until_
#pragma weak shmem_int_wait_until_ = pshmem_int_wait_until_
#pragma weak shmem_long_wait_until_ = pshmem_long_wait_until_
#pragma weak shmem_longlong_wait_until_ = pshmem_longlong_wait_until_
#pragma weak shmem_wait_until_ = pshmem_long_wait_until_

#pragma weak shmem_short_wait_ = pshmem_short_wait_
#pragma weak shmem_int_wait_ = pshmem_int_wait_
#pragma weak shmem_long_wait_ = pshmem_long_wait_
#pragma weak shmem_longlong_wait_ = pshmem_longlong_wait_
#pragma weak shmem_wait_ = pshmem_long_wait_

/*
 * cache flushing
 */

FORTRANIFY_VOID_VOID(pshmem_clear_cache_inv)
FORTRANIFY_VOID_VOID(pshmem_set_cache_inv)
FORTRANIFY_VOID_VOID(pshmem_udcflush)

#define FORTRANIFY_CACHE(Name)			\
    void					\
    FORTRANIFY(Name)(void *target)		\
    {						\
      Name(target);				\
    }

FORTRANIFY_CACHE(pshmem_set_cache_line_inv)
FORTRANIFY_CACHE(pshmem_clear_cache_line_inv)
FORTRANIFY_CACHE(pshmem_udcflush_line)

#pragma weak shmem_clear_cache_inv_ = pshmem_clear_cache_inv_
#pragma weak shmem_clear_cache_line_inv_ = pshmem_clear_cache_line_inv_
#pragma weak shmem_set_cache_inv_ = pshmem_set_cache_inv_
#pragma weak shmem_set_cache_line_inv_ = pshmem_set_cache_line_inv_
#pragma weak shmem_udcflush_line_ = pshmem_udcflush_line_
#pragma weak shmem_udcflush_ = pshmem_udcflush_

/*
 * atomics
 *
 */

void
FORTRANIFY(pshmem_int4_inc)(int *target, int *pe)
{
  pshmem_int_inc(target, *pe);
}

void
FORTRANIFY(pshmem_int8_inc)(long *target, int *pe)
{
  pshmem_long_inc(target, *pe);
}

#pragma weak shmem_int4_inc_ = pshmem_int4_inc_
#pragma weak shmem_int8_inc_ = pshmem_int8_inc_

int
FORTRANIFY(pshmem_int4_finc)(int *target, int *pe)
{
  return pshmem_int_finc(target, *pe);
}

long
FORTRANIFY(pshmem_int8_finc)(long *target, int *pe)
{
  return pshmem_long_finc(target, *pe);
}

#pragma weak shmem_int4_finc_ = pshmem_int4_finc_
#pragma weak shmem_int8_finc_ = pshmem_int8_finc_


void
FORTRANIFY(pshmem_int4_add)(int *target, int *value, int *pe)
{
  pshmem_int_add(target, *value, *pe);
}

void
FORTRANIFY(pshmem_int8_add)(long *target, long *value, int *pe)
{
  pshmem_long_add(target, *value, *pe);
}

#pragma weak shmem_int4_add_ = pshmem_int4_add_
#pragma weak shmem_int8_add_ = pshmem_int8_add_

int
FORTRANIFY(pshmem_int4_fadd)(int *target, int *value, int *pe)
{
  return pshmem_int_fadd(target, *value ,*pe);
}

long
FORTRANIFY(pshmem_int8_fadd)(long *target, long *value, int *pe)
{
  return pshmem_long_fadd(target, *value, *pe);
}

#pragma weak shmem_int4_fadd_ = pshmem_int4_fadd_
#pragma weak shmem_int8_fadd_ = pshmem_int8_fadd_

int
FORTRANIFY(pshmem_int4_swap)(int *target, int *value, int *pe)
{
  return pshmem_int_swap(target, *value, *pe);
}

long
FORTRANIFY(pshmem_int8_swap)(long *target, long *value, int *pe)
{
  return pshmem_long_swap(target, *value, *pe);
}

long
FORTRANIFY(pshmem_swap)(long *target, long *value, int *pe)
{
  return pshmem_long_swap(target, *value, *pe);
}

#pragma weak shmem_int4_swap_ = pshmem_int4_swap_
#pragma weak shmem_int8_swap_ = pshmem_int8_swap_
#pragma weak shmem_swap_ = pshmem_swap_

int
FORTRANIFY(pshmem_int4_cswap)(int *target, int *cond, int *value, int *pe)
{
  return pshmem_int_cswap(target, *cond, *value, *pe);
}

long
FORTRANIFY(pshmem_int8_cswap)(long *target, long *cond, long *value, int *pe)
{
  return pshmem_long_cswap(target, *cond, *value, *pe);
}

#pragma weak shmem_int4_cswap_ = pshmem_int4_cswap_
#pragma weak shmem_int8_cswap_ = pshmem_int8_cswap_


/*
 * broadcasts
 */


void
FORTRANIFY(pshmem_broadcast4)(void *target, const void *source, size_t *nlong,
			      int *PE_root, int *PE_start,
			      int *logPE_stride, int *PE_size,
			      long *pSync)
{
  pshmem_broadcast32(target, source,
		     *nlong, *PE_root, *PE_start, *logPE_stride, *PE_size,
		     pSync);
}

void
FORTRANIFY(pshmem_broadcast8)(void *target, const void *source, size_t *nlong,
			      int *PE_root, int *PE_start,
			      int *logPE_stride, int *PE_size,
			      long *pSync)
{
  pshmem_broadcast32(target, source,
		     *nlong, *PE_root, *PE_start, *logPE_stride, *PE_size,
		     pSync);
}

void
FORTRANIFY(pshmem_broadcast32)(void *target, const void *source, size_t *nlong,
			       int *PE_root, int *PE_start,
			       int *logPE_stride, int *PE_size,
			       long *pSync)
{
  pshmem_broadcast32(target, source,
		     *nlong, *PE_root, *PE_start, *logPE_stride, *PE_size,
		     pSync);
}

void
FORTRANIFY(pshmem_broadcast64)(void *target, const void *source, size_t *nlong,
			       int *PE_root, int *PE_start,
			       int *logPE_stride, int *PE_size,
			       long *pSync)
{
  pshmem_broadcast64(target, source,
		     *nlong, *PE_root, *PE_start, *logPE_stride, *PE_size,
		     pSync);
}

#pragma weak shmem_broadcast4_ = pshmem_broadcast4_
#pragma weak shmem_broadcast8_ = pshmem_broadcast8_
#pragma weak shmem_broadcast32_ = pshmem_broadcast32_
#pragma weak shmem_broadcast64_ = pshmem_broadcast64_



/*
 * fixed collects
 */

void
FORTRANIFY(pshmem_fcollect4)(void *target, const void *source, size_t *nlong,
			     int *PE_start, int *logPE_stride, int *PE_size,
			     long *pSync)
{
  pshmem_fcollect32(target, source,
		    *nlong, *PE_start, *logPE_stride, *PE_size,
		    pSync);
}

void
FORTRANIFY(pshmem_fcollect8)(void *target, const void *source, size_t *nlong,
			     int *PE_start, int *logPE_stride, int *PE_size,
			     long *pSync)
{
  pshmem_fcollect64(target, source,
		    *nlong, *PE_start, *logPE_stride, *PE_size,
		    pSync);
}

#pragma weak shmem_fcollect4_ = pshmem_fcollect4_
#pragma weak shmem_fcollect8_ = pshmem_fcollect8_


/*
 * generalized collects
 */

void
FORTRANIFY(pshmem_collect4)(void *target, const void *source, size_t *nlong,
			    int *PE_start, int *logPE_stride, int *PE_size,
			    long *pSync)
{
  pshmem_collect32(target, source,
		   *nlong, *PE_start, *logPE_stride, *PE_size,
		   pSync);
}

void
FORTRANIFY(pshmem_collect8)(void *target, const void *source, size_t *nlong,
			    int *PE_start, int *logPE_stride, int *PE_size,
			    long *pSync)
{
  pshmem_collect64(target, source,
		   *nlong, *PE_start, *logPE_stride, *PE_size,
		   pSync);
}

#pragma weak shmem_collect4_ = pshmem_collect4_
#pragma weak shmem_collect8_ = pshmem_collect8_


/*
 * reductions
 *
 */

#define REDUCIFY(Op, Fname, Cname, Ctype)				\
  void									\
  FORTRANIFY(pshmem_##Fname##_##Op##_to_all)				\
    (Ctype *target, Ctype *source, int *nreduce,			\
     int *PE_start, int *logPE_stride, int *PE_size,			\
     Ctype *pWrk,							\
     long *pSync)							\
  {									\
    pshmem_##Cname##_##Op##_to_all (target, source,			\
				    *nreduce, *PE_start, *logPE_stride, *PE_size, \
				    pWrk,				\
				    pSync);				\
  }

REDUCIFY(sum, int2, short, short)
REDUCIFY(sum, int4, int, int)
REDUCIFY(sum, int8, long, long)
REDUCIFY(sum, real4, float, float)
REDUCIFY(sum, real8, double, double)
REDUCIFY(sum, real16, longdouble, long double)

REDUCIFY(prod, int2, short, short)
REDUCIFY(prod, int4, int, int)
REDUCIFY(prod, int8, long, long)
REDUCIFY(prod, real4, float, float)
REDUCIFY(prod, real8, double, double)
REDUCIFY(prod, real16, longdouble, long double)

REDUCIFY(max, int2, short, short)
REDUCIFY(max, int4, int, int)
REDUCIFY(max, int8, long, long)
REDUCIFY(max, real4, float, float)
REDUCIFY(max, real8, double, double)
REDUCIFY(max, real16, longdouble, long double)

REDUCIFY(min, int2, short, short)
REDUCIFY(min, int4, int, int)
REDUCIFY(min, int8, long, long)
REDUCIFY(min, real4, float, float)
REDUCIFY(min, real8, double, double)
REDUCIFY(min, real16, longdouble, long double)

REDUCIFY(and, int2, short, short)
REDUCIFY(and, int4, int, int)
REDUCIFY(and, int8, long, long)

REDUCIFY(or, int2, short, short)
REDUCIFY(or, int4, int, int)
REDUCIFY(or, int8, long, long)

REDUCIFY(xor, int2, short, short)
REDUCIFY(xor, int4, int, int)
REDUCIFY(xor, int8, long, long)

REDUCIFY(sum, comp4, complexf, float complex)
REDUCIFY(sum, comp8, complexd, double complex)
REDUCIFY(prod, comp4, complexf, float complex)
REDUCIFY(prod, comp8, complexd, double complex)
REDUCIFY(xor, comp4, complexf, float complex)

#pragma weak shmem_int2_sum_to_all_ = pshmem_int2_sum_to_all_
#pragma weak shmem_int4_sum_to_all_ = pshmem_int4_sum_to_all_
#pragma weak shmem_int8_sum_to_all_ = pshmem_int8_sum_to_all_
#pragma weak shmem_real4_sum_to_all_ = pshmem_real4_sum_to_all_
#pragma weak shmem_real8_sum_to_all_ = pshmem_real8_sum_to_all_
#pragma weak shmem_real16_sum_to_all_ = pshmem_real16_sum_to_all_

#pragma weak shmem_int2_prod_to_all_ = pshmem_int2_prod_to_all_
#pragma weak shmem_int4_prod_to_all_ = pshmem_int4_prod_to_all_
#pragma weak shmem_int8_prod_to_all_ = pshmem_int8_prod_to_all_
#pragma weak shmem_real4_prod_to_all_ = pshmem_real4_prod_to_all_
#pragma weak shmem_real8_prod_to_all_ = pshmem_real8_prod_to_all_
#pragma weak shmem_real16_prod_to_all_ = pshmem_real16_prod_to_all_

#pragma weak shmem_int2_max_to_all_ = pshmem_int2_max_to_all_
#pragma weak shmem_int4_max_to_all_ = pshmem_int4_max_to_all_
#pragma weak shmem_int8_max_to_all_ = pshmem_int8_max_to_all_
#pragma weak shmem_real4_max_to_all_ = pshmem_real4_max_to_all_
#pragma weak shmem_real8_max_to_all_ = pshmem_real8_max_to_all_
#pragma weak shmem_real16_max_to_all_ = pshmem_real16_max_to_all_

#pragma weak shmem_int2_min_to_all_ = pshmem_int2_min_to_all_
#pragma weak shmem_int4_min_to_all_ = pshmem_int4_min_to_all_
#pragma weak shmem_int8_min_to_all_ = pshmem_int8_min_to_all_
#pragma weak shmem_real4_min_to_all_ = pshmem_real4_min_to_all_
#pragma weak shmem_real8_min_to_all_ = pshmem_real8_min_to_all_
#pragma weak shmem_real16_min_to_all_ = pshmem_real16_min_to_all_

#pragma weak shmem_int2_and_to_all_ = pshmem_int2_and_to_all_
#pragma weak shmem_int4_and_to_all_ = pshmem_int4_and_to_all_
#pragma weak shmem_int8_and_to_all_ = pshmem_int8_and_to_all_
#pragma weak shmem_int4_or_to_all_ = pshmem_int4_or_to_all_
#pragma weak shmem_int8_or_to_all_ = pshmem_int8_or_to_all_

#pragma weak shmem_int2_xor_to_all_ = pshmem_int2_xor_to_all_

#pragma weak shmem_comp4_sum_to_all_ = pshmem_comp4_sum_to_all_
#pragma weak shmem_comp8_sum_to_all_ = pshmem_comp8_sum_to_all_
#pragma weak shmem_comp4_prod_to_all_ = pshmem_comp4_prod_to_all_
#pragma weak shmem_comp8_prod_to_all_ = pshmem_comp8_prod_to_all_
#pragma weak shmem_comp4_xor_to_all_ = pshmem_comp4_xor_to_all_


/*
 * locks
 *
 */

void
FORTRANIFY(pshmem_clear_lock)(long *lock)
{
  pshmem_clear_lock(lock);
}

void
FORTRANIFY(pshmem_set_lock)(long *lock)
{
  pshmem_set_lock(lock);
}

int
FORTRANIFY(pshmem_test_lock)(long *lock)
{
  return pshmem_test_lock(lock);
}

#pragma weak shmem_clear_lock_ = pshmem_clear_lock_
#pragma weak shmem_set_lock_ = pshmem_set_lock_
#pragma weak shmem_test_lock_ = pshmem_test_lock_
