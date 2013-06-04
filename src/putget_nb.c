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



#if defined(HAVE_FEATURE_EXPERIMENTAL)

#include "state.h"
#include "trace.h"
#include "utils.h"

#include "shmem.h"

#include "comms/comms.h"

#ifdef HAVE_FEATURE_PSHMEM
# include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */


#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_short_put_nb = pshmem_short_put_nb
#define shmem_short_put_nb pshmem_short_put_nb
#pragma weak shmem_int_put_nb = pshmem_int_put_nb
#define shmem_int_put_nb pshmem_int_put_nb
#pragma weak shmem_long_put_nb = pshmem_long_put_nb
#define shmem_long_put_nb pshmem_long_put_nb
#pragma weak shmem_longdouble_put_nb = pshmem_longdouble_put_nb
#define shmem_longdouble_put_nb pshmem_longdouble_put_nb
#pragma weak shmem_longlong_put_nb = pshmem_longlong_put_nb
#define shmem_longlong_put_nb pshmem_longlong_put_nb
#pragma weak shmem_double_put_nb = pshmem_double_put_nb
#define shmem_double_put_nb pshmem_double_put_nb
#pragma weak shmem_float_put_nb = pshmem_float_put_nb
#define shmem_float_put_nb pshmem_float_put_nb
#pragma weak shmem_put16_nb = pshmem_put16_nb
#define shmem_put16_nb pshmem_put16_nb
#pragma weak shmem_put32_nb = pshmem_put32_nb
#define shmem_put32_nb pshmem_put32_nb
#pragma weak shmem_put64_nb = pshmem_put64_nb
#define shmem_put64_nb pshmem_put64_nb
#pragma weak shmem_put128_nb = pshmem_put128_nb
#define shmem_put128_nb pshmem_put128_nb
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * non-blocking extensions
 */

#define SHMEM_TYPE_PUT_NB(Name, Type)					\
  void *								\
  shmem_##Name##_put_nb (Type *target, const Type *source, size_t nelems, \
			 int pe, void **hp)				\
  {									\
    void *h;								\
    void *rdest;							\
    int typed_nelems = sizeof(Type) * nelems;                           \
    INIT_CHECK ();							\
    PE_RANGE_CHECK (pe);						\
    SYMMETRY_CHECK (target, 1, "shmem_" #Name "_put_nb");		\
    rdest = __shmem_symmetric_addr_lookup (target, pe);            	\
    h = __shmem_comms_put_nb ((Type *) rdest, (Type *) source, typed_nelems, pe); \
    if ((hp != NULL) && (*hp != NULL))					\
      {									\
	*hp = h;							\
      }									\
    return h;								\
  }

SHMEM_TYPE_PUT_NB (short, short);
SHMEM_TYPE_PUT_NB (int, int);
SHMEM_TYPE_PUT_NB (long, long);
SHMEM_TYPE_PUT_NB (longdouble, long double);
SHMEM_TYPE_PUT_NB (longlong, long long);
SHMEM_TYPE_PUT_NB (double, double);
SHMEM_TYPE_PUT_NB (float, float);

void *
shmem_put16_nb (void *dest, const void *src, size_t nelems, int pe, void **hp)
{
  return shmem_short_put_nb (dest, src, nelems, pe, hp);
}

void *
shmem_put32_nb (void *dest, const void *src, size_t nelems, int pe, void **hp)
{
  return shmem_int_put_nb (dest, src, nelems, pe, hp);
}

void *
shmem_put64_nb (void *dest, const void *src, size_t nelems, int pe, void **hp)
{
  return shmem_long_put_nb (dest, src, nelems, pe, hp);
}

void *
shmem_put128_nb (void *dest, const void *src, size_t nelems, int pe, void **hp)
{
  return shmem_longdouble_put_nb (dest, src, nelems, pe, hp);
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_putmem_nb = pshmem_putmem_nb
#define shmem_putmem_nb pshmem_putmem_nb
#pragma weak shmem_put_nb = pshmem_put_nb
#define shmem_put_nb pshmem_put_nb
#endif /* HAVE_FEATURE_PSHMEM */

void *
shmem_putmem_nb (void *target, const void *source, size_t nelems,
		 int pe, void **hp)
{
  void *h;
  void *rdest;
  INIT_CHECK ();
  PE_RANGE_CHECK (pe);
  SYMMETRY_CHECK (target, 1, "shmem_putmem_nb");
  rdest = __shmem_symmetric_addr_lookup (target, pe);
  h = __shmem_comms_put_nb (rdest, (void *) source, nelems, pe);
  if ((hp != NULL) && (*hp != NULL))
    {
      *hp = h;
    }
  return h;
}

void *
shmem_put_nb (long *target, const long *source, size_t nelems,
	      int pe, void **hp)
{
  return shmem_long_put_nb (target, source, nelems, pe, hp);
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_short_get_nb = pshmem_short_get_nb
#define shmem_short_get_nb pshmem_short_get_nb
#pragma weak shmem_int_get_nb = pshmem_int_get_nb
#define shmem_int_get_nb pshmem_int_get_nb
#pragma weak shmem_long_get_nb = pshmem_long_get_nb
#define shmem_long_get_nb pshmem_long_get_nb
#pragma weak shmem_longdouble_get_nb = pshmem_longdouble_get_nb
#define shmem_longdouble_get_nb pshmem_longdouble_get_nb
#pragma weak shmem_longlong_get_nb = pshmem_longlong_get_nb
#define shmem_longlong_get_nb pshmem_longlong_get_nb
#pragma weak shmem_double_get_nb = pshmem_double_get_nb
#define shmem_double_get_nb pshmem_double_get_nb
#pragma weak shmem_float_get_nb = pshmem_float_get_nb
#define shmem_float_get_nb pshmem_float_get_nb
#pragma weak shmem_get16_nb = pshmem_get16_nb
#define shmem_get16_nb pshmem_get16_nb
#pragma weak shmem_get32_nb = pshmem_get32_nb
#define shmem_get32_nb pshmem_get32_nb
#pragma weak shmem_get64_nb = pshmem_get64_nb
#define shmem_get64_nb pshmem_get64_nb
#pragma weak shmem_get128_nb = pshmem_get128_nb
#define shmem_get128_nb pshmem_get128_nb
#endif /* HAVE_FEATURE_PSHMEM */

#define SHMEM_TYPE_GET_NB(Name, Type)					\
  void *								\
  shmem_##Name##_get_nb (Type *target, const Type *source, size_t nelems, \
			 int pe, void **hp)				\
  {									\
    void *h;								\
    void *rsrc;								\
    int typed_nelems = sizeof(Type) * nelems;                           \
    INIT_CHECK ();							\
    PE_RANGE_CHECK (pe);						\
    SYMMETRY_CHECK (source, 2, "shmem_" #Name "_get_nb");		\
    rsrc = __shmem_symmetric_addr_lookup ((void *) source, pe);     	\
    h = __shmem_comms_get_nb ((Type *) target, (Type *) rsrc, typed_nelems, pe); \
    if ((hp != NULL) && (*hp != NULL))					\
      {									\
	*hp = h;							\
      }									\
    return h;								\
  }

SHMEM_TYPE_GET_NB (short, short);
SHMEM_TYPE_GET_NB (int, int);
SHMEM_TYPE_GET_NB (long, long);
SHMEM_TYPE_GET_NB (longdouble, long double);
SHMEM_TYPE_GET_NB (longlong, long long);
SHMEM_TYPE_GET_NB (double, double);
SHMEM_TYPE_GET_NB (float, float);

void *
shmem_get16_nb (void *dest, const void *src, size_t nelems, int pe, void **hp)
{
  return shmem_short_get_nb (dest, src, nelems, pe, hp);
}

void *
shmem_get32_nb (void *dest, const void *src, size_t nelems, int pe, void **hp)
{
  return shmem_int_get_nb (dest, src, nelems, pe, hp);
}

void *
shmem_get64_nb (void *dest, const void *src, size_t nelems, int pe, void **hp)
{
  return shmem_long_get_nb (dest, src, nelems, pe, hp);
}

void *
shmem_get128_nb (void *dest, const void *src, size_t nelems, int pe, void **hp)
{
  return shmem_longdouble_get_nb (dest, src, nelems, pe, hp);
}


#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_getmem_nb = pshmem_getmem_nb
#define shmem_getmem_nb pshmem_getmem_nb
#pragma weak shmem_get_nb = pshmem_get_nb
#define shmem_get_nb pshmem_get_nb
#endif /* HAVE_FEATURE_PSHMEM */

void *
shmem_getmem_nb (void *target, const void *source, size_t nelems,
		 int pe, void **hp)
{
  void *h;
  void *rsrc;
  INIT_CHECK ();
  PE_RANGE_CHECK (pe);
  SYMMETRY_CHECK (source, 2, "shmem_getmem_nb");
  rsrc = __shmem_symmetric_addr_lookup ((void *) source, pe);
  h = __shmem_comms_get_nb (target, (void *) rsrc, nelems, pe);
  if ((hp != NULL) && (*hp != NULL))
    {
      *hp = h;
    }
  return h;
}

void *
shmem_get_nb (long *target, const long *source, size_t nelems, int pe, void **hp)
{
  return shmem_long_get_nb (target, source, nelems, pe, hp);
}

#ifdef HAVE_FEATURE_PSHMEM
#pragma weak shmem_wait_nb = pshmem_wait_nb
#define shmem_wait_nb pshmem_wait_nb
#pragma weak shmem_test_nb = pshmem_test_nb
#define shmem_test_nb pshmem_test_nb
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * Wait for handle to be completed
 */

void shmem_wait_nb (void *h)
{
  __shmem_comms_wait_nb (h);
}

/**
 * Test whether handle has been completed
 */

int
shmem_test_nb (void *h)
{
  return __shmem_comms_test_nb (h);
}

#endif /* HAVE_FEATURE_EXPERIMENTAL */
