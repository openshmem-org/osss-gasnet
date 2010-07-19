#include <semaphore.h>

#include "state.h"
#include "shmem.h"

static sem_t swap_lock;
static sem_t cswap_lock;

void
__shmem_atomic_init(void)
{
  sem_init(&swap_lock, 0, 1);
  sem_init(&cswap_lock, 0, 1);
}

void
__shmem_atomic_finalize(void)
{
  sem_destroy(&swap_lock);
  sem_destroy(&cswap_lock);
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
 *
 *
 * or use Active Messages???
 *
 * & atomic intrinsic for local-local
 */

#define SHMEM_TYPE_SWAP(Name, Type)					\
  Type									\
  shmem_##Name##_swap(Type *target, Type value, int pe)			\
  {									\
    Type retval;							\
    if (__state.mype == pe) {						\
      sem_wait(&swap_lock);						\
      retval = *target;							\
      *target = value;							\
      sem_post(&swap_lock);						\
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

SHMEM_TYPE_SWAP(short, short)
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
      sem_wait(&cswap_lock);						\
      retval = *target;							\
      if (cond == retval) {						\
	*target = value;						\
      }									\
      sem_post(&cswap_lock);						\
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

SHMEM_TYPE_CSWAP(short, short)
SHMEM_TYPE_CSWAP(int, int)
SHMEM_TYPE_CSWAP(long, long)
SHMEM_TYPE_CSWAP(longlong, long long)
SHMEM_TYPE_CSWAP(double, double)
SHMEM_TYPE_CSWAP(float, float)

_Pragma("weak shmem_cswap=shmem_long_cswap")
