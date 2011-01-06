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

#define SHMEM_TYPE_FADD(Name, Type)					\
  /* @api@ */								\
  Type									\
  shmem_##Name##_fadd(Type *target, Type value, int pe)			\
  {									\
    Type retval;							\
    if (__state.mype == pe) {						\
      retval = __sync_fetch_and_add(target, value);			\
    }									\
    else {								\
      __comms_fadd_request(target, &value, sizeof(value), pe, &retval);	\
    }									\
    return retval;							\
  }
  
SHMEM_TYPE_FADD(int, int)
SHMEM_TYPE_FADD(long, long)
SHMEM_TYPE_FADD(longlong, long long)

#ifdef HAVE_PSHMEM_SUPPORT
#pragma weak pshmem_int_fadd = shmem_int_fadd
#pragma weak pshmem_long_fadd = shmem_long_fadd
#pragma weak pshmem_longlong_fadd = shmem_longlong_fadd
#endif /* HAVE_PSHMEM_SUPPORT */

#define SHMEM_TYPE_FINC(Name, Type)					\
  /* @api@ */								\
  Type									\
  shmem_##Name##_finc(Type *target, int pe)				\
  {									\
    Type retval;							\
    if (__state.mype == pe) {						\
      retval = __sync_fetch_and_add(target, (Type) 1);			\
    }									\
    else {								\
      __comms_finc_request(target, sizeof(retval), pe, &retval);	\
    }									\
    return retval;							\
  }

#if 0
#define SHMEM_TYPE_FINC(Name, Type)			\
  /* @api@ */						\
  Type							\
  shmem_##Name##_finc(Type *target, int pe)		\
  {							\
    return shmem_##Name##_fadd(target, (Type) 1, pe);	\
  }
#endif

SHMEM_TYPE_FINC(int, int)
SHMEM_TYPE_FINC(long, long)
SHMEM_TYPE_FINC(longlong, long long)

#ifdef HAVE_PSHMEM_SUPPORT
#pragma weak pshmem_int_finc = shmem_int_finc
#pragma weak pshmem_long_finc = shmem_long_finc
#pragma weak pshmem_longlong_finc = shmem_longlong_finc
#endif /* HAVE_PSHMEM_SUPPORT */

/*
 * remote increment/add
 *
 * Let's do this the naughty way for now!  Just throw away the
 * finc/fadd result
 *
 * TODO: dedicated handler to avoid moving the unwanted former value
 * around
 *
 */

#define SHMEM_TYPE_ADD(Name, Type)					\
  /* @api@ */								\
  void									\
  shmem_##Name##_add(Type *target, Type value, int pe)			\
  {									\
    (void) shmem_##Name##_fadd(target, value, pe);			\
  }
  
SHMEM_TYPE_ADD(int, int)
SHMEM_TYPE_ADD(long, long)
SHMEM_TYPE_ADD(longlong, long long)

#define SHMEM_TYPE_INC(Name, Type)			\
  /* @api@ */						\
  void							\
  shmem_##Name##_inc(Type *target, int pe)		\
  {							\
    (void) shmem_##Name##_finc(target, pe);		\
  }

SHMEM_TYPE_INC(int, int)
SHMEM_TYPE_INC(long, long)
SHMEM_TYPE_INC(longlong, long long)

#ifdef HAVE_PSHMEM_SUPPORT
#pragma weak pshmem_int_add = shmem_int_add
#pragma weak pshmem_long_add = shmem_long_add
#pragma weak pshmem_longlong_add = shmem_longlong_add

#pragma weak pshmem_int_inc = shmem_int_inc
#pragma weak pshmem_long_inc = shmem_long_inc
#pragma weak pshmem_longlong_inc = shmem_longlong_inc
#endif /* HAVE_PSHMEM_SUPPORT */
