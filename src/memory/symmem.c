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



#include <stdio.h>
#include <assert.h>
#include <sys/types.h>

#include "state.h"
#include "comms.h"
#include "trace.h"
#include "memalloc.h"
#include "utils.h"

#include "shmem.h"

#ifdef HAVE_FEATURE_PSHMEM
# include "pshmem.h"
#endif /* HAVE_FEATURE_PSHMEM */

/*
 * Not present in SGI any more.
 * I'm going to leave it in the code, but we'll ignore it
 * and it's removed from shmem.h
 *
 */
long malloc_error = SHMEM_MALLOC_OK;	/* exposed for error codes */


/**
 * check that all PEs see the same shmalloc size: return first
 * mis-matching PE id if there's a mis-match, return -1 to record
 * correct symmetry (no offending PE)
 */

int
__shmalloc_symmetry_check (size_t size)
{
  int pe;
  int any_failed_pe = -1;
  long shmalloc_received_size;
  long *shmalloc_remote_size;

  /* record for everyone else to see */
  shmalloc_remote_size =
    (long *) __shmem_mem_alloc (sizeof (*shmalloc_remote_size));
  if (shmalloc_remote_size == (long *) NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: couldn't allocate memory for symmetry check");
      /* NOT REACHED */
    }
  *shmalloc_remote_size = size;
  shmem_barrier_all ();

  malloc_error = SHMEM_MALLOC_OK;

  /*
   * everyone checks everyone else's sizes, barf if mis-match
   *
   * TODO: probably some kind of Eureka! optimization opportunity here
   *
   */
  for (pe = 0; pe < GET_STATE (numpes); pe += 1)
    {
      if (pe == GET_STATE (mype))
	{
	  continue;
	}
      shmalloc_received_size = shmem_long_g (shmalloc_remote_size, pe);
      if (shmalloc_received_size != size)
	{
	  __shmem_trace (SHMEM_LOG_NOTICE,
			 "shmalloc expected %ld, but saw %ld on PE %d",
			 size, shmalloc_received_size, pe);
	  malloc_error = SHMEM_MALLOC_SYMMSIZE_FAILED;
	  any_failed_pe = pe;
	  break;
	  /* NOT REACHED */
	}
    }
  /* make sure everyone is here before freeing things */
  shmem_barrier_all ();
  __shmem_mem_free (shmalloc_remote_size);
  return any_failed_pe;
}

/**
 * this call avoids the symmetry check that the real shmalloc() has to
 * do and is thus cheaper.  Intended for internal use when we know in
 * advance the supplied size is symmetric.
 *
 */

void *
__shmalloc_no_check (size_t size)
{
  void *addr;

  addr = __shmem_mem_alloc (size);

  if (addr == (void *) NULL)
    {
      __shmem_trace (SHMEM_LOG_NOTICE, "shmalloc(%ld bytes) failed", size);
      malloc_error = SHMEM_MALLOC_FAIL;
    }
  else
    {
      malloc_error = SHMEM_MALLOC_OK;
    }

  __shmem_trace (SHMEM_LOG_MEMORY, "shmalloc(%ld bytes) @ %p", size, addr);

  return addr;
}


#ifdef HAVE_FEATURE_PSHMEM
# pragma weak shmalloc = pshmalloc
# define shmalloc pshmalloc
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * Symmetrically allocate "size" byte of memory across all PEs
 */

void *
shmalloc (size_t size)
{
  void *addr;

  INIT_CHECK ();

  if (__shmalloc_symmetry_check (size) != -1)
    {
      malloc_error = SHMEM_MALLOC_SYMMSIZE_FAILED;
      return (void *) NULL;
      /* NOT REACHED */
    }

  __shmem_trace (SHMEM_LOG_MEMORY,
		 "shmalloc(%ld bytes) passed symmetry check",
		 size);

  addr = __shmalloc_no_check (size);

  shmem_barrier_all ();

  return addr;
}

#ifdef HAVE_FEATURE_PSHMEM
# pragma weak shfree = pshfree
# define shfree pshfree
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * Symmetrically free previously allocated memory
 */

void
shfree (void *addr)
{
  INIT_CHECK ();

  shmem_barrier_all ();

  if (addr == (void *) NULL)
    {
      __shmem_trace (SHMEM_LOG_MEMORY,
		     "address passed to shfree() already null");
      malloc_error = SHMEM_MALLOC_ALREADY_FREE;
      return;
      /* NOT REACHED */
    }

  __shmem_trace (SHMEM_LOG_MEMORY,
		 "shfree(%p) in pool @ %p", addr, __shmem_mem_base ());

  __shmem_mem_free (addr);

  malloc_error = SHMEM_MALLOC_OK;
}

#ifdef HAVE_FEATURE_PSHMEM
# pragma weak shrealloc = pshrealloc
# define shrealloc pshrealloc
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * Resize previously allocated symmetric memory
 */

void *
shrealloc (void *addr, size_t size)
{
  void *newaddr;

  INIT_CHECK ();

  if (addr == (void *) NULL)
    {
      __shmem_trace (SHMEM_LOG_MEMORY,
		     "address passed to shrealloc() is null, handing to shmalloc()");
      return shmalloc (size);
      /* NOT REACHED */
    }

  if (size == 0)
    {
      __shmem_trace (SHMEM_LOG_MEMORY,
		     "size passed to shrealloc() is 0, handing to shfree()");
      shfree (addr);
      return (void *) NULL;
      /* NOT REACHED */
    }

  if (__shmalloc_symmetry_check (size) != -1)
    {
      malloc_error = SHMEM_MALLOC_SYMMSIZE_FAILED;
      return (void *) NULL;
      /* NOT REACHED */
    }

  newaddr = __shmem_mem_realloc (addr, size);

  if (newaddr == (void *) NULL)
    {
      __shmem_trace (SHMEM_LOG_MEMORY,
		     "shrealloc(%ld bytes) failed @ original address %p",
		     size, addr);
      malloc_error = SHMEM_MALLOC_REALLOC_FAILED;
    }
  else
    {
      malloc_error = SHMEM_MALLOC_OK;
    }

  shmem_barrier_all ();

  return newaddr;
}

#ifdef HAVE_FEATURE_PSHMEM
# pragma weak shmemalign = pshmemalign
# define shmemalign pshmemalign
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * The shmemalign function allocates a block in the symmetric heap that
 * has a byte alignment specified by the alignment argument.
 */

void *
shmemalign (size_t alignment, size_t size)
{
  void *addr;

  INIT_CHECK ();

  if (__shmalloc_symmetry_check (size) != -1)
    {
      malloc_error = SHMEM_MALLOC_SYMMSIZE_FAILED;
      return (void *) NULL;
      /* NOT REACHED */
    }

  addr = __shmem_mem_align (alignment, size);

  if (addr == (void *) NULL)
    {
      __shmem_trace (SHMEM_LOG_MEMORY,
		     "shmem_memalign(%ld bytes) couldn't realign to %ld",
		     size, alignment);
      malloc_error = SHMEM_MALLOC_MEMALIGN_FAILED;
    }
  else
    {
      malloc_error = SHMEM_MALLOC_OK;
    }

  shmem_barrier_all ();

  return addr;
}

/**
 * readable error message for error code "e"
 */

typedef struct
{
  long code;
  char *msg;
} malloc_error_code_t;

static malloc_error_code_t error_table[] =
  {
    {SHMEM_MALLOC_OK,
     "no symmetric memory allocation error"},
    {SHMEM_MALLOC_FAIL,
     "symmetric memory allocation failed"},
    {SHMEM_MALLOC_ALREADY_FREE,
     "attempt to free already null symmetric memory address"},
    {SHMEM_MALLOC_MEMALIGN_FAILED,
     "attempt to align symmetric memory address failed"},
    {SHMEM_MALLOC_REALLOC_FAILED,
     "attempt to reallocate symmetric memory address failed"},
    {SHMEM_MALLOC_SYMMSIZE_FAILED,
     "asymmetric sizes passed to symmetric memory allocator"},
    {SHMEM_MALLOC_BAD_SIZE,
     "size of data to allocate can not be negative"},
    {SHMEM_MALLOC_NOT_ALIGNED,
     "address is not block-aligned"},
    {SHMEM_MALLOC_NOT_IN_SYMM_HEAP,
     "address falls outside of symmetric heap"},
  };
static const int nerrors = TABLE_SIZE (error_table);

#ifdef HAVE_FEATURE_PSHMEM
extern char *sherror (void);
# pragma weak sherror = psherror
# define sherror psherror
#endif /* HAVE_FEATURE_PSHMEM */

/**
 * Return human-readable error message
 */

char *
sherror (void)
{
  malloc_error_code_t *etp = error_table;
  int i;

  INIT_CHECK ();

  for (i = 0; i < nerrors; i += 1)
    {
      if (malloc_error == etp->code)
	{
	  return etp->msg;
	  /* NOT REACHED */
	}
      etp += 1;
    }

  return "unknown memory error";
}
