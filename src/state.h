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



#ifndef _STATE_H
#define _STATE_H 1

#include <sys/types.h>
#include <sys/param.h>
#include <sys/utsname.h>
#include <sys/time.h>

/*
 * known PE states
 */

typedef enum
{
  PE_UNINITIALIZED = 0,		/* start like this */
  PE_UNKNOWN,			/* for when we have no information yet */
  PE_RUNNING,			/* after start_pes() */
  PE_SHUTDOWN,			/* clean exit */
  PE_FAILED,			/* something went wrong */
} pe_status_t;

/*
 * translate PE state to human description
 */

extern const char *__shmem_state_as_string (pe_status_t s);

/*
 * per-PE state structure
 */

typedef struct
{
  pe_status_t pe_status;	/* up and running yet? */

  int numpes;			/* # of processing elements */
  int mype;			/* rank of this processing element */

  size_t heapsize;		/* size of symmetric heap (bytes) */

  struct itimerval ping_timeout;	/* wait for remote PE to ack ping */

  struct utsname loc;		/* location information (currently not used) */

  char exe_name[MAXPATHLEN];	/* real name of executable */
  int exe_fd;			/* file descriptor of executable */

} state_t;

/*
 * the per-PE state
 */

extern state_t __state;

/*
 * set/get state variables
 */

#define SET_STATE(var, val)   ( __state.var = val )
#define GET_STATE(var)        ( __state.var )

#endif /* _STATE_H */
