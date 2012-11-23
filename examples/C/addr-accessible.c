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



/*
 * test whether various types of variables are accessible
 *
 * oshrun -np 2 ./accessible.x
 *
 *
 */

#include <stdio.h>
#include <shmem.h>

static int
check_it (void *addr)
{
  return shmem_addr_accessible (addr, 1);
}

long global_target;
static int static_target;

int
main (int argc, char *argv[])
{
  long local_target;
  int *shm_target;
  char *msg = "OK";
  int me;

  start_pes (0);
  me = _my_pe ();

  shm_target = (int *) shmalloc (sizeof (int));

  if (me == 0)
    {

      if (!check_it (&global_target))
	{			/* long global: yes */
	  msg = "FAIL (global long)";
	}
      if (!check_it (&static_target))
	{			/* static int global: yes */
	  msg = "FAIL (static int)";
	}
      if (check_it (&local_target))
	{			/* main() stack: no  */
	  msg = "FAIL (stack variable)";
	}
      if (!check_it (shm_target))
	{			/* shmalloc: yes */
	  msg = "FAIL (shmalloc)";
	}

      printf ("%s\n", msg);

    }

  shfree (shm_target);

  return 0;
}
