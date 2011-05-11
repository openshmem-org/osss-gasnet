#include <stdio.h>

#include "state.h"
#include "comms.h"
#include "trace.h"

#include "mpp/shmem.h"

/*
 * this waits for the variable to change but also dispatches
 * other put/get traffic in the meantime
 */

#define SHMEM_WAIT_LOOP_FRAGMENT(Type, Var, Op, CmpValue)	\
  while ( ! ( (* ( volatile Type *)(Var)) Op CmpValue) ) {	\
    __shmem_comms_pause();					\
  }

/*
 * wait_until with operator dispatchers, type-parameterized
 */

#define SHMEM_TYPE_WAIT_UNTIL(Name, Type)				\
  /* @api@ */								\
  void									\
  pshmem_##Name##_wait_until(Type *ivar, int cmp, Type cmp_value)	\
  {									\
    if (cmp == SHMEM_CMP_EQ) {						\
      SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, ==, cmp_value);		\
    }									\
    else if (cmp == SHMEM_CMP_NE) {					\
      SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, !=, cmp_value);		\
    }									\
    else if (cmp == SHMEM_CMP_GT) {					\
      SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, >, cmp_value);		\
    }									\
    else if (cmp == SHMEM_CMP_LE) {					\
      SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, <=, cmp_value);		\
    }									\
    else if (cmp == SHMEM_CMP_LT) {					\
      SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, <, cmp_value);		\
    }									\
    else if (cmp == SHMEM_CMP_GE) {					\
      SHMEM_WAIT_LOOP_FRAGMENT(Type, ivar, >=, cmp_value);		\
    }									\
    else {								\
      __shmem_trace(SHMEM_LOG_FATAL,					\
		    "unknown operator (code %d) in shmem_%s_wait_until()", \
		    cmp,						\
		    #Name						\
		    );							\
    }									\
  }

SHMEM_TYPE_WAIT_UNTIL(short, short)
SHMEM_TYPE_WAIT_UNTIL(int, int)
SHMEM_TYPE_WAIT_UNTIL(long, long)
SHMEM_TYPE_WAIT_UNTIL(longlong, long long)

#pragma weak pshmem_wait_until = pshmem_long_wait_until

/*
 * wait is just wait_until with equality test
 */
#define SHMEM_TYPE_WAIT(Name, Type)					\
  /* @api@ */								\
  void									\
  pshmem_##Name##_wait(Type *ivar, Type cmp_value)			\
  {									\
    pshmem_##Name##_wait_until(ivar, SHMEM_CMP_NE, cmp_value);		\
  }

SHMEM_TYPE_WAIT(short, short)
SHMEM_TYPE_WAIT(int, int)
SHMEM_TYPE_WAIT(long, long)
SHMEM_TYPE_WAIT(longlong, long long)

#pragma weak pshmem_wait = pshmem_long_wait

#pragma weak shmem_short_wait_until = pshmem_short_wait_until
#pragma weak shmem_int_wait_until = pshmem_int_wait_until
#pragma weak shmem_long_wait_until = pshmem_long_wait_until
#pragma weak shmem_longlong_wait_until = pshmem_longlong_wait_until
#pragma weak shmem_wait_until = pshmem_wait_until
#pragma weak shmem_short_wait = pshmem_short_wait
#pragma weak shmem_int_wait = pshmem_int_wait
#pragma weak shmem_long_wait = pshmem_long_wait
#pragma weak shmem_longlong_wait = pshmem_longlong_wait
#pragma weak shmem_wait = pshmem_wait
