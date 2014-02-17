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



#include "state.h"
#include "trace.h"
#include "utils.h"

#include "shmem.h"

#ifdef HAVE_FEATURE_PSHMEM
# include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */

/*
 *
 */

static
inline
int
mype_helper (void)
{
  INIT_CHECK ();
  return GET_STATE (mype);
}

#ifdef HAVE_FEATURE_PSHMEM
# pragma weak _my_pe = p_my_pe
# define _my_pe p_my_pe
# pragma weak shmem_my_pe = pshmem_my_pe
# define shmem_my_pe pshmem_my_pe
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * \brief These routines return the "rank" or identity of the calling PE
 *
 * \b Synopsis:
 *
 * - C/C++:
 * \code
 *   int _my_pe (void);
 *   int shmem_my_pe (void);
 * \endcode
 *
 * - Fortran:
 * \code
 *   INTEGER I
 *
 *   I = MY_PE ()
 * \endcode
 *
 * \b Effect:
 *
 * None.
 *
 * \return Rank of calling PE
 *
 * \b Deprecation:
 *
 * _my_pes() and MY_PES() are deprecated as of OpenSHMEM 1.1.
 *
 */

int
_my_pe (void)
{
  return mype_helper ();
}

int
shmem_my_pe (void)
{
  return mype_helper ();
}

/*
 *
 */

static
inline
int
numpes_helper (void)
{
  INIT_CHECK ();
  return GET_STATE (numpes);
}

#ifdef HAVE_FEATURE_PSHMEM
# pragma weak _num_pes = p_num_pes
# define _num_pes p_num_pes
# pragma weak shmem_n_pes = pshmem_n_pes
# define shmem_n_pes pshmem_n_pes
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * \brief These routines return the number of PEs in the program
 *
 * \b Synopsis:
 *
 * - C/C++:
 * \code
 *   int _num_pes (void);
 *   int shmem_n_pes (void);
 * \endcode
 *
 * - Fortran:
 * \code
 *   INTEGER I
 *
 *   I = NUM_PES ()
 *   I = SHMEM_N_PES ()
 * \endcode
 *
 * \b Effect:
 *
 * None.
 *
 * \return Number of PEs in program
 *
 * \b Deprecation:
 *
 * _num_pes() and NUM_PES() are deprecated as of OpenSHMEM 1.1.
 *
 */

int
_num_pes (void)
{
  return numpes_helper ();
}

int
shmem_n_pes (void)
{
  return numpes_helper ();
}

#if defined(HAVE_FEATURE_EXPERIMENTAL)

/*
 *
 */

#ifdef HAVE_FEATURE_PSHMEM
extern char *shmem_nodename (void); /* ! API */
# pragma weak shmem_nodename = pshmem_nodename
# define shmem_nodename pshmem_nodename
#endif /* HAVE_FEATURE_PSHMEM */

char *
shmem_nodename (void)
{
  INIT_CHECK ();
  return GET_STATE (loc.nodename);
}

#endif /* HAVE_FEATURE_EXPERIMENTAL */
