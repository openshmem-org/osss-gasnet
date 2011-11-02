/*
 *
 * Copyright (c) 2011, University of Houston System and Oak Ridge National
 * Loboratory.
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
 *   National Loboratory nor the names of its contributors may be used to
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
#include "utils.h"

/*
 * initialize the PE's state (this is all we need to initialize)
 */

state_t __state = {
  .pe_status = PE_UNINITIALIZED
};

/*
 * PE status and its human description
 */

struct state_desc
{
  pe_status_t s;
  const char *desc;
};

/*
 * table of known PE status
 */

static struct state_desc d[] = {
  {PE_UNINITIALIZED, "PE has not been initialized yet"},
  {PE_UNKNOWN, "I have no information about PE"},
  {PE_RUNNING, "PE is running"},
  {PE_SHUTDOWN, "PE has been cleanly shut down"},
  {PE_FAILED, "PE has failed"},
};
static const int nd = TABLE_SIZE (d);

/*
 * translate PE status to human description
 */

const char *
__shmem_state_as_string (pe_status_t s)
{
  struct state_desc *dp = d;
  int i;

  for (i = 0; i < nd; i += 1)
    {
      if (s == dp->s)
	{
	  return dp->desc;
	  /* NOT REACHED */
	}
      dp += 1;
    }

  return "unknown state";
}
