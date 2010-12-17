#include <sys/types.h>

#include "state.h"
#include "comms.h"
#include "shmem.h"

/*
 * placeholders: no init/final required (so far)
 */

void
__shmem_atomic_init(void)
{
}

void
__shmem_atomic_finalize(void)
{
}

/*
 * shmem_swap  performs  an  atomic  swap operation. It writes value "value"
 * into "target" on processing element (PE)  pe  and  returns  the  previous
 * contents of target as an atomic operation.
 */

static int64_t
__atomic_xchg64(volatile int64_t *p, int64_t new_value)
{
  __asm__ volatile ("lock xchg %1, %0" : 
                "+m" (*p), "+r" (new_value) : : 
                "memory");
  return new_value;
}

static int64_t
__atomic_cmpxchg64(volatile int64_t *p, int64_t old_value, int64_t new_value)
{
  __asm__ volatile ("lock cmpxchg %2, %0" : 
                "+m" (*p), "+a" (old_value) :
                "r" (new_value) : 
                "memory");
  return old_value;
}

/*
 * shmem_swap performs an atomic swap operation. It writes value value
 * into target on processing element (PE) pe and returns the previous
 * contents of target as an atomic operation.
 */

#define SHMEM_TYPE_SWAP(Name, Type)					\
  /* @api@ */								\
  Type									\
  shmem_##Name##_swap(Type *target, Type value, int pe)			\
  {									\
    Type retval;							\
    if (__state.mype == pe) {						\
      retval = __atomic_xchg64((volatile int64_t *)target, value);	\
    }									\
    else {								\
      __comms_swap_request(target, &value, sizeof(value), pe, &retval);	\
    }									\
    return retval;							\
  }

/* SHMEM_TYPE_SWAP(short, short) */
SHMEM_TYPE_SWAP(int, int)
SHMEM_TYPE_SWAP(long, long)
SHMEM_TYPE_SWAP(longlong, long long)
SHMEM_TYPE_SWAP(double, double)
SHMEM_TYPE_SWAP(float, float)

#pragma weak shmem_swap = shmem_long_swap



/*
 * The conditional swap routines conditionally update a target data
 * object on an arbitrary processing element (PE) and return the prior
 * contents of the data object in one atomic operation.
 */

#define SHMEM_TYPE_CSWAP(Name, Type)					\
  /* @api@ */								\
  Type									\
  shmem_##Name##_cswap(Type *target, Type cond, Type value, int pe)	\
  {									\
    Type retval;							\
    if (__state.mype == pe) {						\
      retval = __atomic_cmpxchg64((volatile int64_t *)target, cond, value); \
    }									\
    else {								\
      __comms_cswap_request(target, &cond, &value, sizeof(value), pe, &retval); \
    }									\
    return retval;							\
  }

SHMEM_TYPE_CSWAP(int, int)
SHMEM_TYPE_CSWAP(long, long)
SHMEM_TYPE_CSWAP(longlong, long long)

#pragma weak shmem_cswap = shmem_long_cswap

#ifdef HAVE_PSHMEM_SUPPORT
#pragma weak pshmem_int_swap = shmem_int_swap
#pragma weak pshmem_long_swap = shmem_long_swap
#pragma weak pshmem_longlong_swap = shmem_longlong_swap
#pragma weak pshmem_float_swap = shmem_float_swap
#pragma weak pshmem_double_swap = shmem_double_swap
#pragma weak pshmem_swap = shmem_long_swap

#pragma weak pshmem_int_cswap = shmem_int_cswap
#pragma weak pshmem_long_cswap = shmem_long_cswap
#pragma weak pshmem_longlong_cswap = shmem_longlong_cswap
#pragma weak pshmem_cswap = shmem_long_cswap
#endif /* HAVE_PSHMEM_SUPPORT */
