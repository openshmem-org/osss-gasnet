/*
 *
 * Copyright (c) 2011 - 2013
 *   University of Houston System and Oak Ridge National Laboratory.
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * o Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * 
 * o Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 
 * o Neither the name of the University of Houston System, Oak Ridge
 *   National Laboratory nor the names of its contributors may be used to
 *   endorse or promote products derived from this software without specific
 *   prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */



#include <stdio.h>

#include "state.h"
#include "trace.h"

#include "shmem.h"

#include "comms/comms.h"

#ifdef HAVE_FEATURE_PSHMEM
# include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */


/**
 * this waits for the variable to change but also dispatches
 * other put/get traffic in the meantime
 */

#define SHMEM_WAIT_LOOP_FRAGMENT(Type, Var, Op, CmpValue)	\
  while ( ! ( (* ( volatile Type *) (Var)) Op CmpValue) )	\
    {								\
      __shmem_comms_service ();					\
    }


#ifdef HAVE_FEATURE_PSHMEM
# pragma weak shmem_short_wait_until = pshmem_short_wait_until
# define shmem_short_wait_until pshmem_short_wait_until
# pragma weak shmem_int_wait_until = pshmem_int_wait_until
# define shmem_int_wait_until pshmem_int_wait_until
# pragma weak shmem_long_wait_until = pshmem_long_wait_until
# define shmem_long_wait_until pshmem_long_wait_until
# pragma weak shmem_longlong_wait_until = pshmem_longlong_wait_until
# define shmem_longlong_wait_until pshmem_longlong_wait_until
# pragma weak shmem_wait_until = pshmem_wait_until
# define shmem_wait_until pshmem_wait_until
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * wait_until with operator dispatchers, type-parameterized.
 * NB the _SHMEM_CMP values are identical to the SHMEM_CMP ones.
 */

#define SHMEM_TYPE_WAIT_UNTIL(Name, Type)				\
  void									\
  shmem_##Name##_wait_until (Type *ivar, int cmp, Type cmp_value)	\
  {									\
    switch (cmp) {							\
    case SHMEM_CMP_EQ:							\
      SHMEM_WAIT_LOOP_FRAGMENT (Type, ivar, ==, cmp_value);		\
      break;								\
    case SHMEM_CMP_NE:							\
      SHMEM_WAIT_LOOP_FRAGMENT (Type, ivar, !=, cmp_value);		\
      break;								\
    case SHMEM_CMP_GT:							\
      SHMEM_WAIT_LOOP_FRAGMENT (Type, ivar, >, cmp_value);		\
      break;								\
    case SHMEM_CMP_LE:							\
      SHMEM_WAIT_LOOP_FRAGMENT (Type, ivar, <=, cmp_value);		\
      break;								\
    case SHMEM_CMP_LT:							\
      SHMEM_WAIT_LOOP_FRAGMENT (Type, ivar, <, cmp_value);		\
      break;								\
    case SHMEM_CMP_GE:							\
      SHMEM_WAIT_LOOP_FRAGMENT (Type, ivar, >=, cmp_value);		\
      break;								\
    default:								\
      __shmem_trace (SHMEM_LOG_FATAL,					\
		     "unknown operator (code %d) in shmem_%s_wait_until()", \
		     cmp,						\
		     #Name						\
		     );							\
    }									\
  }

SHMEM_TYPE_WAIT_UNTIL (short, short);
SHMEM_TYPE_WAIT_UNTIL (int, int);
SHMEM_TYPE_WAIT_UNTIL (long, long);
SHMEM_TYPE_WAIT_UNTIL (longlong, long long);

/**
 * and a special case for the untyped call
 */

void
shmem_wait_until (long *ivar, int cmp, long cmp_value)
{
  shmem_long_wait_until (ivar, cmp, cmp_value);
}

#ifdef HAVE_FEATURE_PSHMEM
# pragma weak shmem_short_wait = pshmem_short_wait
# define shmem_short_wait pshmem_short_wait
# pragma weak shmem_int_wait = pshmem_int_wait
# define shmem_int_wait pshmem_int_wait
# pragma weak shmem_long_wait = pshmem_long_wait
# define shmem_long_wait pshmem_long_wait
# pragma weak shmem_longlong_wait = pshmem_longlong_wait
# define shmem_longlong_wait pshmem_longlong_wait
# pragma weak shmem_wait = pshmem_wait
# define shmem_wait pshmem_wait
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * wait is just wait_until with inequality/change test
 */

#define SHMEM_TYPE_WAIT(Name, Type)					\
  void									\
  shmem_##Name##_wait(Type *ivar, Type cmp_value)			\
  {									\
    shmem_##Name##_wait_until (ivar, SHMEM_CMP_NE, cmp_value);		\
  }

SHMEM_TYPE_WAIT (short, short);
SHMEM_TYPE_WAIT (int, int);
SHMEM_TYPE_WAIT (long, long);
SHMEM_TYPE_WAIT (longlong, long long);

/**
 * and a special case for the untyped call
 */

void
shmem_wait (long *ivar, long cmp_value)
{
  shmem_long_wait (ivar, cmp_value);
}
