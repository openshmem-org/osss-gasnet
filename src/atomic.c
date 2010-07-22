#include <sys/types.h>

#include "state.h"

#include "shmem.h"

/*
 * placeholders: no init/final required (so far)
 */

__inline__ void
__shmem_atomic_init(void)
{
}

__inline__ void
__shmem_atomic_finalize(void)
{
}

/*
 * shmem_swap  performs  an  atomic  swap operation. It writes value "value"
 * into "target" on processing element (PE)  pe  and  returns  the  previous
 * contents of target as an atomic operation.
 */

/*
 * TODO: using put/get?  probably stupid way...
 * get old target from pe;
 * write value to target on pe;
 * return old target
 * UGH, barriers: should use "wait" for point-to-point sync
 *
 *
 * or use Active Messages???
 *
 * & atomic intrinsic for local-local
 */

static __inline__ int64_t
__atomic_xchg64(volatile int64_t *p, int64_t new_value)
{
  asm volatile ("lock xchg %1, %0" : 
                "+m" (*p), "+r" (new_value) : : 
                "memory");
  return new_value;
}

static __inline__ int64_t
__atomic_cmpxchg64(volatile int64_t *p, int64_t old_value, int64_t new_value)
{
  asm volatile ("lock cmpxchg %2, %0" : 
                "+m" (*p), "+a" (old_value) :
                "r" (new_value) : 
                "memory");
  return old_value;
}

#define SHMEM_TYPE_SWAP(Name, Type)					\
  Type									\
  shmem_##Name##_swap(Type *target, Type value, int pe)			\
  {									\
    Type retval;							\
    if (__state.mype == pe) {						\
      retval = __atomic_xchg64((volatile int64_t *)target, value);	\
    }									\
    else {								\
      Type *oldtarget = (Type *) shmalloc( sizeof(*oldtarget) );	\
      shmem_##Name##_get(oldtarget, target, 1, pe);			\
      shmem_barrier_all();						\
      shmem_##Name##_put(target, &value, 1, pe);			\
      shmem_barrier_all();						\
      retval = *oldtarget;						\
      shfree(oldtarget);						\
      return retval;							\
    }									\
  }

/* SHMEM_TYPE_SWAP(short, short) */
SHMEM_TYPE_SWAP(int, int)
SHMEM_TYPE_SWAP(long, long)
SHMEM_TYPE_SWAP(longlong, long long)
SHMEM_TYPE_SWAP(double, double)
SHMEM_TYPE_SWAP(float, float)

_Pragma("weak shmem_swap=shmem_long_swap") 

#define SHMEM_TYPE_CSWAP(Name, Type)					\
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
      shmem_barrier_all();						\
      if (cond == *oldtarget) {						\
	shmem_##Name##_put(target, &value, 1, pe);			\
	shmem_barrier_all();						\
      }									\
      retval = *oldtarget;						\
      shfree(oldtarget);						\
      return retval;							\
    }									\
  }

SHMEM_TYPE_CSWAP(int, int)
SHMEM_TYPE_CSWAP(long, long)
SHMEM_TYPE_CSWAP(longlong, long long)

_Pragma("weak shmem_cswap=shmem_long_cswap")
