/*
 *
 * Copyright (c) 2011, 2012, University of Houston System and Oak Ridge National
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



#ifndef _UTILS_H
#define _UTILS_H 1

#ifdef DEBUG

#include <stdio.h>
#include <stdlib.h>

#include "state.h"

#define IF_DEBUGGING(x) do { x ; } while (0)

#else /* ! DEBUG */

#define IF_DEBUGGING(x)

#endif /* DEBUG */

/*
 * if we haven't initialized through start_pes() then try to do
 * something constructive.  Obviously can't use __shmem_trace()
 * because nothing has been initialized.
 *
 */

#define INIT_CHECK()							\
  IF_DEBUGGING(								\
	       if (GET_STATE(pe_status) != PE_RUNNING) {		\
		 fprintf(stderr,					\
			 "Error: OpenSHMEM library has not been initialized\n" \
			 );						\
		 exit(1);						\
		 /* NOT REACHED */					\
	       }							\
	       )

/*
 * make sure a target PE is within the assigned range
 *
 */

#include "trace.h"
#include "query.h"

#define PE_RANGE_CHECK(p)						\
  IF_DEBUGGING(								\
	       {							\
		 const int bot_pe = 0;					\
		 const int top_pe = GET_STATE(numpes) - 1;		\
		 if (pe < bot_pe || pe > top_pe) {			\
		   __shmem_trace(SHMEM_LOG_FATAL,			\
				 "Target PE %d not within allocated range %d .. %d", \
				 pe, bot_pe, top_pe			\
				 );					\
		   /* NOT REACHED */					\
		 }							\
	       }							\
	       )

/*
 * check for symmetry of required addresses
 *
 */

#include "symmtest.h"

#define SYMMETRY_CHECK(addr, argpos, subrname)				\
  IF_DEBUGGING(								\
	       if (! __shmem_is_symmetric( (void *) (addr))) {		\
		 __shmem_trace(SHMEM_LOG_FATAL,				\
			       "%s(), argument #%d at %p is not symmetric", \
			       subrname,				\
			       argpos,					\
			       addr					\
			       );					\
		 /* NOT REACHED */					\
	       }							\
	       )

/*
 * how many elements in array T?
 *
 */
#define TABLE_SIZE(T) ( sizeof(T) / sizeof((T)[0]) )

#endif /* _UTILS_H */
