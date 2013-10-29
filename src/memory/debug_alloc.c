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


#ifdef HAVE_FEATURE_DEBUG

# include "uthash.h"

# include "trace.h"

# include "debug_alloc.h"

static alloc_table_t *atp = NULL; /* our allocation hash table */

alloc_table_t *
debug_alloc_new (void *a, size_t s)
{
  alloc_table_t *at = (alloc_table_t *) malloc (sizeof (*at));

  if (at == (alloc_table_t *) NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: out of memory allocating address/size record"
		     );
      /* NOT REACHED */
    }
  at->addr = a;
  at->size = s;
  return at;
}

void *
debug_alloc_find (void *a)
{
  alloc_table_t *at = NULL;	/* entry corresponding to "a" */

  HASH_FIND_PTR (atp, &a, at);
  return at;
}

int
debug_alloc_check (void *a)
{
  alloc_table_t *tmp;
  alloc_table_t *s;

  HASH_ITER (hh, atp, s, tmp) {
    void *lo = s->addr;
    void *hi = lo + s->size - 1;

    if ( (lo <= a) && (a <= hi) )
      {
	return 1;
      }
  }
  return 0;
}

void
debug_alloc_add (void *a, size_t s)
{
  alloc_table_t *at = debug_alloc_new (a, s);

  HASH_ADD_PTR (atp, addr, at);
}

void
debug_alloc_del (void *a)
{
  alloc_table_t *at = debug_alloc_find (a);

  if (at == (alloc_table_t *) NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: no hash table entry for address %p",
		     a
		     );
      /* NOT REACHED */
    }
  HASH_DEL (atp, at);

  free (at);
}

void
debug_alloc_replace (void *a, size_t s)
{
#if 0
  alloc_table_t *at = debug_alloc_new (a, s);
  HASH_REPLACE_PTR (atp, a, at);
#else
  debug_alloc_del (a);
  debug_alloc_add (a, s);
#endif
}

void
debug_alloc_dump (void)
{
  alloc_table_t *tmp;
  alloc_table_t *s;

  HASH_ITER (hh, atp, s, tmp) {
    __shmem_trace (SHMEM_LOG_MEMORY,
		   "addr = %p, size = %ld", s->addr, s->size
		   );
  }
}

#endif /* HAVE_FEATURE_DEBUG */
