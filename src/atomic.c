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
 * TODO: spin on retval being set by reply handler
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
      retval = * (Type *) __comms_swap_request(target, &value, sizeof(value), pe); \
    }									\
    return retval;							\
  }

/* SHMEM_TYPE_SWAP(short, short) */
SHMEM_TYPE_SWAP(int, int)
SHMEM_TYPE_SWAP(long, long)
SHMEM_TYPE_SWAP(longlong, long long)
SHMEM_TYPE_SWAP(double, double)
SHMEM_TYPE_SWAP(float, float)

_Pragma("weak shmem_swap=shmem_long_swap") 

/*
 * this is an utterly stupid attempt.  Just so you know :-)
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
      Type *oldtarget = (Type *) shmalloc( sizeof(*oldtarget) );	\
      shmem_##Name##_get(oldtarget, target, 1, pe);			\
      if (cond == *oldtarget) {						\
	shmem_##Name##_put(target, &value, 1, pe);			\
	shmem_barrier_all();						\
      }									\
      retval = *oldtarget;						\
      shmem_barrier_all();						\
      shfree(oldtarget);						\
      return retval;							\
    }									\
  }

SHMEM_TYPE_CSWAP(int, int)
SHMEM_TYPE_CSWAP(long, long)
SHMEM_TYPE_CSWAP(longlong, long long)

_Pragma("weak shmem_cswap=shmem_long_cswap")

#ifdef HAVE_PSHMEM_SUPPORT
_Pragma("weak pshmem_int_swap=shmem_int_swap")
_Pragma("weak pshmem_long_swap=shmem_long_swap")
_Pragma("weak pshmem_longlong_swap=shmem_longlong_swap")
_Pragma("weak pshmem_float_swap=shmem_float_swap")
_Pragma("weak pshmem_double_swap=shmem_double_swap")

_Pragma("weak pshmem_int_cswap=shmem_int_cswap")
_Pragma("weak pshmem_long_cswap=shmem_long_cswap")
_Pragma("weak pshmem_longlong_cswap=shmem_longlong_cswap")
#endif /* HAVE_PSHMEM_SUPPORT */
