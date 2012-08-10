/*
 *
 * Copyright (c) 2011, 2012
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



#pragma weak _my_pe = p_my_pe
#define _my_pe p_my_pe
#pragma weak shmem_my_pe = pshmem_my_pe
#define shmem_my_pe pshmem_my_pe

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
 */

int
_my_pe (void)
{
  INIT_CHECK();
  return GET_STATE(mype);
}

int
shmem_my_pe (void)
{
  return _my_pe ();
}


#pragma weak _num_pes = p_num_pes
#define _num_pes p_num_pes
#pragma weak shmem_n_pes = pshmem_n_pes
#define shmem_n_pes pshmem_n_pes

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
 */

int
_num_pes (void)
{
  INIT_CHECK();
  return GET_STATE(numpes);
}

int
shmem_n_pes (void)
{
  return _num_pes ();
}



#pragma weak shmem_nodename = pshmem_nodename
#define shmem_nodename pshmem_nodename

char *
shmem_nodename (void)
{
  INIT_CHECK ();
  return GET_STATE (loc.nodename);
}


#pragma weak shmem_version = pshmem_version
#define shmem_version pshmem_version

/**
 * OpenSHMEM has a major.minor release number.  Return 0 if
 * successful, -1 otherwise
 *
 */

/* @api@ */
int
shmem_version (int *major, int *minor)
{
#if ! defined(SHMEM_MAJOR_VERSION) && ! defined(SHMEM_MINOR_VERSION)
  return -1;
#endif /* ! SHMEM_MAJOR_VERSION && ! SHMEM_MINOR_VERSION */
  *major = SHMEM_MAJOR_VERSION;
  *minor = SHMEM_MINOR_VERSION;
  return 0;
}
