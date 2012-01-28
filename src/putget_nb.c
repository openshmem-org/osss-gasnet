/*
 *
 * Copyright (c) 2011, University of Houston System and Oak Ridge National
 * Laboratory.
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
  pshmem_##Name##_put_nb(Type *target, const Type *source, size_t nelems, int pe) \
  {									\
    INIT_CHECK();							\
    PE_RANGE_CHECK(pe);							\
    return __shmem_comms_##Name##_put_nb(target, source, nelems, pe);	\
  }

SHMEM_TYPE_PUT_NB (short, short)
SHMEM_TYPE_PUT_NB (int, int)
SHMEM_TYPE_PUT_NB (long, long)
SHMEM_TYPE_PUT_NB (longdouble, long double)
SHMEM_TYPE_PUT_NB (longlong, long long)
SHMEM_TYPE_PUT_NB (double, double)
SHMEM_TYPE_PUT_NB (float, float)

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
  pshmem_##Name##_get_nb(Type *target, const Type *source, size_t nelems, int pe) \
  {									\
    INIT_CHECK();							\
    PE_RANGE_CHECK(pe);							\
    return __shmem_comms_##Name##_get_nb(target, source, nelems, pe);	\
  }

SHMEM_TYPE_GET_NB (short, short)
SHMEM_TYPE_GET_NB (int, int)
SHMEM_TYPE_GET_NB (long, long)
SHMEM_TYPE_GET_NB (longdouble, long double)
SHMEM_TYPE_GET_NB (longlong, long long)
SHMEM_TYPE_GET_NB (double, double)
SHMEM_TYPE_GET_NB (float, float)

#pragma weak shmem_short_get_nb = pshmem_short_get_nb
#pragma weak shmem_int_get_nb = pshmem_int_get_nb
#pragma weak shmem_long_get_nb = pshmem_long_get_nb
#pragma weak shmem_longdouble_get_nb = pshmem_longdouble_get_nb
#pragma weak shmem_longlong_get_nb = pshmem_longlong_get_nb
#pragma weak shmem_double_get_nb = pshmem_double_get_nb
#pragma weak shmem_float_get_nb = pshmem_float_get_nb

/* @api@ */
void pshmem_wait_nb (void *h)
{
  __shmem_comms_wait_nb (h);
}

/* @api@ */
int
pshmem_test_nb (void *h)
{
  return __shmem_comms_test_nb (h);
}

#pragma weak shmem_wait_nb = pshmem_wait_nb
#pragma weak shmem_test_nb = pshmem_test_nb

#endif /* HAVE_PUTGET_NB */
