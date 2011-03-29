#if defined(HAVE_PUTGET_NB)

#include "state.h"
#include "comms.h"
#include "trace.h"
#include "utils.h"

#include "pshmem.h"

/*
 * non-blocking extensions
 */

#define SHMEM_TYPE_PUT_NB(Name, Type)					\
  /* @api@ */								\
  void *								\
  pshmem_##Name##_put_nb(Type *target, const Type *source, size_t len, int pe) \
  {									\
    INIT_CHECK();							\
    PE_RANGE_CHECK(pe);							\
    return __shmem_comms_##Name##_put_nb(target, source, len, pe);	\
  }

SHMEM_TYPE_PUT_NB(short, short)
SHMEM_TYPE_PUT_NB(int, int)
SHMEM_TYPE_PUT_NB(long, long)
SHMEM_TYPE_PUT_NB(longdouble, long double)
SHMEM_TYPE_PUT_NB(longlong, long long)
SHMEM_TYPE_PUT_NB(double, double)
SHMEM_TYPE_PUT_NB(float, float)

#pragma weak shmem_short_put_nb = pshmem_short_put_nb
#pragma weak shmem_int_put_nb = pshmem_int_put_nb
#pragma weak shmem_long_put_nb = pshmem_long_put_nb
#pragma weak shmem_longdouble_put_nb = pshmem_longdouble_put_nb
#pragma weak shmem_longlong_put_nb = pshmem_longlong_put_nb
#pragma weak shmem_double_put_nb = pshmem_double_put_nb
#pragma weak shmem_float_put_nb = pshmem_float_put_nb


#define SHMEM_TYPE_GET_NB(Name, Type)					\
  /* @api@ */								\
  void *								\
  pshmem_##Name##_get_nb(Type *target, const Type *source, size_t len, int pe) \
  {									\
    INIT_CHECK();							\
    PE_RANGE_CHECK(pe);							\
    return __shmem_comms_##Name##_get_nb(target, source, len, pe);	\
  }

SHMEM_TYPE_GET_NB(short, short)
SHMEM_TYPE_GET_NB(int, int)
SHMEM_TYPE_GET_NB(long, long)
SHMEM_TYPE_GET_NB(longdouble, long double)
SHMEM_TYPE_GET_NB(longlong, long long)
SHMEM_TYPE_GET_NB(double, double)
SHMEM_TYPE_GET_NB(float, float)

#pragma weak shmem_short_get_nb = pshmem_short_get_nb
#pragma weak shmem_int_get_nb = pshmem_int_get_nb
#pragma weak shmem_long_get_nb = pshmem_long_get_nb
#pragma weak shmem_longdouble_get_nb = pshmem_longdouble_get_nb
#pragma weak shmem_longlong_get_nb = pshmem_longlong_get_nb
#pragma weak shmem_double_get_nb = pshmem_double_get_nb
#pragma weak shmem_float_get_nb = pshmem_float_get_nb


/* @api@ */
void
pshmem_wait_nb(void *h)
{
  __shmem_comms_wait_nb(h);
}

/* @api@ */
int
pshmem_test_nb(void *h)
{
  return __shmem_comms_test_nb(h);
}

#pragma weak shmem_wait_nb = pshmem_wait_nb
#pragma weak shmem_test_nb = pshmem_test_nb

#endif /* HAVE_PUTGET_NB */
