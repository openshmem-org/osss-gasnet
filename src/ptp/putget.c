/*
 *
 * Copyright (c) 2011 - 2014
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



#include <stdio.h>		/* NULL                           */
#include <string.h>		/* memcpy()                       */
#include <sys/types.h>		/* size_t                         */

#include "state.h"
#include "globalvar.h"
#include "updown.h"
#include "trace.h"
#include "utils.h"
#include "symmtest.h"

#include "shmem.h"

#include "comms/comms.h"

#ifdef HAVE_FEATURE_PSHMEM
# include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */

#ifdef HAVE_FEATURE_PSHMEM
extern void shmem_complexf_put (COMPLEXIFY (float) * dest,
				const COMPLEXIFY (float) * src,
				size_t nelems, int pe); /* ! API */
extern void shmem_complexd_put (COMPLEXIFY (double) * dest,
				const COMPLEXIFY (double) * src,
				size_t nelems, int pe); /* ! API */
# pragma weak shmem_short_put = pshmem_short_put
# define shmem_short_put pshmem_short_put
# pragma weak shmem_int_put = pshmem_int_put
# define shmem_int_put pshmem_int_put
# pragma weak shmem_long_put = pshmem_long_put
# define shmem_long_put pshmem_long_put
# pragma weak shmem_longdouble_put = pshmem_longdouble_put
# define shmem_longdouble_put pshmem_longdouble_put
# pragma weak shmem_longlong_put = pshmem_longlong_put
# define shmem_longlong_put pshmem_longlong_put
# pragma weak shmem_double_put = pshmem_double_put
# define shmem_double_put pshmem_double_put
# pragma weak shmem_float_put = pshmem_float_put
# define shmem_float_put pshmem_float_put
# pragma weak shmem_complexf_put = pshmem_complexf_put
# define shmem_complexf_put pshmem_complexf_put
# pragma weak shmem_complexd_put = pshmem_complexd_put
# define shmem_complexd_put pshmem_complexd_put
# pragma weak shmem_putmem = pshmem_putmem
# define shmem_putmem pshmem_putmem
# pragma weak shmem_put32 = pshmem_put32
# define shmem_put32 pshmem_put32
# pragma weak shmem_put64 = pshmem_put64
# define shmem_put64 pshmem_put64
# pragma weak shmem_put128 = pshmem_put128
# define shmem_put128 pshmem_put128
/* # pragma weak pshmem_put = pshmem_long_put */
/* # pragma weak shmem_put = pshmem_put */
#endif /* HAVE_FEATURE_PSHMEM */

/*
 * do typed puts.  NB any short-circuits or address translations are
 * now deferred to the comms layer
 */

#define SHMEM_TYPE_PUT(Name, Type)					\
  void									\
  shmem_##Name##_put (Type *dest, const Type *src, size_t nelems, int pe) \
  {									\
    const int typed_nelems = sizeof (Type) * nelems;			\
    INIT_CHECK ();							\
    SYMMETRY_CHECK (dest, 1, "shmem_" #Name "_put");			\
    PE_RANGE_CHECK (pe, 4);						\
    __shmem_comms_put (dest, (Type *) src, typed_nelems, pe);		\
  }

SHMEM_TYPE_PUT (char, char);
SHMEM_TYPE_PUT (short, short);
SHMEM_TYPE_PUT (int, int);
SHMEM_TYPE_PUT (long, long);
SHMEM_TYPE_PUT (longlong, long long);
SHMEM_TYPE_PUT (longdouble, long double);
SHMEM_TYPE_PUT (double, double);
SHMEM_TYPE_PUT (float, float);
SHMEM_TYPE_PUT (complexf, COMPLEXIFY (float));
SHMEM_TYPE_PUT (complexd, COMPLEXIFY (double));

void
shmem_put32 (void *dest, const void *src, size_t nelems, int pe)
{
  shmem_int_put (dest, src, nelems, pe);
}

void
shmem_put64 (void *dest, const void *src, size_t nelems, int pe)
{
  shmem_long_put (dest, src, nelems, pe);
}

void
shmem_put128 (void *dest, const void *src, size_t nelems, int pe)
{
  shmem_longdouble_put (dest, src, nelems, pe);
}

void
shmem_putmem (void *dest, const void *src, size_t nelems, int pe)
{
  INIT_CHECK ();
  SYMMETRY_CHECK (dest, 1, "shmem_putmem");
  PE_RANGE_CHECK (pe, 4);
  __shmem_comms_put_bulk (dest, (void *) src, nelems, pe);
}


#ifdef HAVE_FEATURE_PSHMEM
extern void shmem_complexf_get (COMPLEXIFY (float) * dest,
				const COMPLEXIFY (float) * src,
				size_t nelems, int pe); /* ! API */
extern void shmem_complexd_get (COMPLEXIFY (double) * dest,
				const COMPLEXIFY (double) * src,
				size_t nelems, int pe); /* ! API */
# pragma weak shmem_short_get = pshmem_short_get
# define shmem_short_get pshmem_short_get
# pragma weak shmem_int_get = pshmem_int_get
# define shmem_int_get pshmem_int_get
# pragma weak shmem_long_get = pshmem_long_get
# define shmem_long_get pshmem_long_get
# pragma weak shmem_longdouble_get = pshmem_longdouble_get
# define shmem_longdouble_get pshmem_longdouble_get
# pragma weak shmem_longlong_get = pshmem_longlong_get
# define shmem_longlong_get pshmem_longlong_get
# pragma weak shmem_double_get = pshmem_double_get
# define shmem_double_get pshmem_double_get
# pragma weak shmem_float_get = pshmem_float_get
# define shmem_float_get pshmem_float_get
# pragma weak shmem_complexf_get = pshmem_complexf_get
# define shmem_complexf_get pshmem_complexf_get
# pragma weak shmem_complexd_get = pshmem_complexd_get
# define shmem_complexd_get pshmem_complexd_get
# pragma weak shmem_getmem = pshmem_getmem
# define shmem_getmem pshmem_getmem
# pragma weak shmem_get32 = pshmem_get32
# define shmem_get32 pshmem_get32
# pragma weak shmem_get64 = pshmem_get64
# define shmem_get64 pshmem_get64
# pragma weak shmem_get128 = pshmem_get128
# define shmem_get128 pshmem_get128
/* # pragma weak pshmem_get = pshmem_long_get */
/* # pragma weak shmem_get = pshmem_get */
#endif /* HAVE_FEATURE_PSHMEM */

#define SHMEM_TYPE_GET(Name, Type)					\
  void									\
  shmem_##Name##_get (Type *dest, const Type *src, size_t nelems, int pe) \
  {									\
    const int typed_nelems = sizeof (Type) * nelems;			\
    INIT_CHECK ();							\
    SYMMETRY_CHECK (src, 2, "shmem_" #Name "_get");			\
    PE_RANGE_CHECK (pe, 4);						\
    __shmem_comms_get(dest, (void *) src, typed_nelems, pe);		\
  }

SHMEM_TYPE_GET (char, char);
SHMEM_TYPE_GET (short, short);
SHMEM_TYPE_GET (int, int);
SHMEM_TYPE_GET (long, long);
SHMEM_TYPE_GET (longdouble, long double);
SHMEM_TYPE_GET (longlong, long long);
SHMEM_TYPE_GET (double, double);
SHMEM_TYPE_GET (float, float);
SHMEM_TYPE_GET (complexf, COMPLEXIFY (float));
SHMEM_TYPE_GET (complexd, COMPLEXIFY (double));;

void
shmem_get32 (void *dest, const void *src, size_t nelems, int pe)
{
  shmem_int_get (dest, src, nelems, pe);
}

void
shmem_get64 (void *dest, const void *src, size_t nelems, int pe)
{
  shmem_long_get (dest, src, nelems, pe);
}

void
shmem_get128 (void *dest, const void *src, size_t nelems, int pe)
{
  shmem_longdouble_get (dest, src, nelems, pe);
}

void
shmem_getmem (void *dest, const void *src, size_t nelems, int pe)
{
  INIT_CHECK ();
  SYMMETRY_CHECK (src, 2, "shmem_getmem");
  PE_RANGE_CHECK (pe, 4);
  __shmem_comms_get_bulk (dest, (void *) src, nelems, pe);
}


#ifdef HAVE_FEATURE_PSHMEM
# pragma weak shmem_char_p = pshmem_char_p
# define shmem_char_p pshmem_char_p
# pragma weak shmem_short_p = pshmem_short_p
# define shmem_short_p pshmem_short_p
# pragma weak shmem_int_p = pshmem_int_p
# define shmem_int_p pshmem_int_p
# pragma weak shmem_long_p = pshmem_long_p
# define shmem_long_p pshmem_long_p
# pragma weak shmem_longdouble_p = pshmem_longdouble_p
# define shmem_longdouble_p pshmem_longdouble_p
# pragma weak shmem_longlong_p = pshmem_longlong_p
# define shmem_longlong_p pshmem_longlong_p
# pragma weak shmem_double_p = pshmem_double_p
# define shmem_double_p pshmem_double_p
# pragma weak shmem_float_p = pshmem_float_p
# define shmem_float_p pshmem_float_p
#endif /* HAVE_FEATURE_PSHMEM */

/*
 * gasnet_(get|get)_val can't handle bigger types..
 */
#define SHMEM_TYPE_P_WRAPPER(Name, Type)				\
  void									\
  shmem_##Name##_p (Type *dest, Type value, int pe)			\
  {									\
    shmem_##Name##_put (dest, &value, 1, pe);				\
  }

SHMEM_TYPE_P_WRAPPER (float, float);
SHMEM_TYPE_P_WRAPPER (double, double);
SHMEM_TYPE_P_WRAPPER (longdouble, long double);
SHMEM_TYPE_P_WRAPPER (longlong, long long);
SHMEM_TYPE_P_WRAPPER (char, char);
SHMEM_TYPE_P_WRAPPER (short, short);
SHMEM_TYPE_P_WRAPPER (int, int);
SHMEM_TYPE_P_WRAPPER (long, long);
SHMEM_TYPE_P_WRAPPER (complexf, COMPLEXIFY (float));



#ifdef HAVE_FEATURE_PSHMEM
# pragma weak shmem_char_g = pshmem_char_g
# define shmem_char_g pshmem_char_g
# pragma weak shmem_short_g = pshmem_short_g
# define shmem_short_g pshmem_short_g
# pragma weak shmem_int_g = pshmem_int_g
# define shmem_int_g pshmem_int_g
# pragma weak shmem_long_g = pshmem_long_g
# define shmem_long_g pshmem_long_g
# pragma weak shmem_longdouble_g = pshmem_longdouble_g
# define shmem_longdouble_g pshmem_longdouble_g
# pragma weak shmem_longlong_g = pshmem_longlong_g
# define shmem_longlong_g pshmem_longlong_g
# pragma weak shmem_double_g = pshmem_double_g
# define shmem_double_g pshmem_double_g
# pragma weak shmem_float_g = pshmem_float_g
# define shmem_float_g pshmem_float_g
#endif /* HAVE_FEATURE_PSHMEM */

#define SHMEM_TYPE_G_WRAPPER(Name, Type)				\
  Type									\
  shmem_##Name##_g (Type *addr, int pe)					\
  {									\
    Type retval;							\
    shmem_##Name##_get (&retval, addr, 1, pe);				\
    return retval;							\
  }

SHMEM_TYPE_G_WRAPPER (float, float);
SHMEM_TYPE_G_WRAPPER (double, double);
SHMEM_TYPE_G_WRAPPER (longlong, long long);
SHMEM_TYPE_G_WRAPPER (longdouble, long double);
SHMEM_TYPE_G_WRAPPER (char, char);
SHMEM_TYPE_G_WRAPPER (short, short);
SHMEM_TYPE_G_WRAPPER (int, int);
SHMEM_TYPE_G_WRAPPER (long, long);
SHMEM_TYPE_G_WRAPPER (complexf, COMPLEXIFY (float));
