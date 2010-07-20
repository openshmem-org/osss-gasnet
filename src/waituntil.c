#include <stdio.h>

#include "gasnet_safe.h"

#include "shmem.h"
#include "state.h"

/*
 * this waits for the variable to change but also dispatches
 * other put/get traffic in the meantime
 */

#define SHMEM_WAIT_LOOP_FRAGMENT(Type, Var, Op)		\
  do {							\
    gasnet_AMPoll();					\
  } while ( *((volatile Type *) Var) Op cmp_value)

/*
 * wait_util with operator dispatchers, type-parameterized
 */

#define SHMEM_TYPE_WAIT_UNTIL(Name, Type)				\
  void									\
  shmem_##Name##_wait_until(Type *ivar, int cmp, Type cmp_value)	\
  {									\
    GASNET_BEGIN_FUNCTION();						\
    if (cmp == SHMEM_CMP_EQ) {						\
      SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, ==);				\
    }									\
    else if (cmp == SHMEM_CMP_NE) {					\
      SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, !=);				\
    }									\
    else if (cmp == SHMEM_CMP_GT) {					\
      SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, >);				\
    }									\
    else if (cmp == SHMEM_CMP_LE) {					\
      SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, <=);				\
    }									\
    else if (cmp == SHMEM_CMP_LT) {					\
      SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, <);				\
    }									\
    else if (cmp == SHMEM_CMP_GE) {					\
      SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, >=);				\
    }									\
    else {								\
      ;  /* error handler goes here */					\
    }									\
  }

SHMEM_TYPE_WAIT_UNTIL(short, short)
SHMEM_TYPE_WAIT_UNTIL(int, int)
SHMEM_TYPE_WAIT_UNTIL(long, long)
SHMEM_TYPE_WAIT_UNTIL(longlong, long long)

_Pragma("weak shmem_wait_until=shmem_long_wait_until")

/*
 * wait is just wait_until with equality test
 */
#define SHMEM_TYPE_WAIT(Name, Type)				\
  void								\
  shmem_##Name##_wait(Type *ivar, Type cmp_value)		\
  {								\
    shmem_##Name##_wait_until(ivar, SHMEM_CMP_EQ, cmp_value);	\
  }

SHMEM_TYPE_WAIT(short, short)
SHMEM_TYPE_WAIT(int, int)
SHMEM_TYPE_WAIT(long, long)
SHMEM_TYPE_WAIT(longlong, long long)

_Pragma("weak shmem_wait=shmem_long_wait")
