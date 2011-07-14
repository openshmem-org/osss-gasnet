/* (c) 2011 University of Houston System.  All rights reserved. */


#include <sys/types.h>

#include "symmem.h"
#include "utils.h"
#include "trace.h"

#include "mpp/pshmem.h"

#include "fortran-common.h"

/*
 * Fortran symmetric memory operations
 */

/*
 * SYNOPSIS
 *   POINTER (addr, A(1))
 *   INTEGER (length, errcode, abort)
 *   CALL SHPALLOC(addr, length, errcode, abort)
 * 
 * DESCRIPTION
 *   SHPALLOC  allocates a block of memory from the program's symmetric heap
 *   that is greater than or equal  to  the  size  requested.   To  maintain
 *   symmetric  heap  consistency,  all PEs in an program must call SHPALLOC
 *   with the same value of length; if any  processing  elements  (PEs)  are
 *   missing, the program will hang.
 */

void
FORTRANIFY(pshpalloc)(long *addr, size_t *length, long *errcode, int *abort)
{
  long *symm_addr;

  INIT_CHECK();

  __shmem_trace(SHMEM_LOG_MEMORY,
		"addr = %p, length = %d, errcode = %d, abort = %d",
		addr, *length, *errcode, *abort
		);

  symm_addr = (long *) pshmalloc(*length * sizeof(long));

  /* pass back status code */
  *errcode = malloc_error;

  /* if malloc succeeded, nothing else to do */
  if (malloc_error == SHMEM_MALLOC_OK) {
    addr = symm_addr;
    return;
    /* NOT REACHED */
  }

  /* failed somehow, we might have to abort */
  __shmem_trace(*abort ? SHMEM_LOG_FATAL : SHMEM_LOG_MEMORY,
		"shpalloc() got non-symmetric memory sizes"
		);
  /* MAYBE NOT REACHED */

  addr = (long *) NULL;
}

/*
 * SYNOPSIS
 *   POINTER (addr, A(1))
 *   INTEGER errcode, abort
 *   CALL SHPDEALLC(addr, errcode, abort)
 * 
 * 
 * DESCRIPTION
 *   SHPDEALLC  returns  a block of memory (allocated using SHPALLOC) to the
 *   list of available space in the symmetric heap.  To  maintain  symmetric
 *   heap  consistency, all processing elements (PEs) in a program must call
 *   SHPDEALLC with the same value of addr; if  any  PEs  are  missing,  the
 *   program hangs.
 */

void
FORTRANIFY(pshpdeallc)(void *addr, long *errcode, int *abort)
{
  INIT_CHECK();

  pshfree(addr);

  /* pass back status code */
  *errcode = malloc_error;

  /* if malloc succeeded, nothing else to do */
  if (malloc_error == SHMEM_MALLOC_OK) {
    return;
    /* NOT REACHED */
  }

  /* failed somehow, we might have to abort */
  __shmem_trace(*abort ? SHMEM_LOG_FATAL : SHMEM_LOG_MEMORY,
		"shpdeallc() failed: %s",
		sherror()
		);
  /* MAYBE NOT REACHED */
}

/*
 * SYNOPSIS
 *   POINTER (addr, A(1))
 *   INTEGER length, status, abort
 *   CALL SHPCLMOVE (addr, length, status, abort)
 * 
 * 
 * DESCRIPTION
 *   The SHPCLMOVE function either extends a symmetric  heap  block  if  the
 *   block  is  followed by a large enough free block or copies the contents
 *   of the existing block to a larger  block  and  returns  a  status  code
 *   indicating that the block was moved.  This function also can reduce the
 *   size of a block if the new length is less than  the  old  length.   All
 *   processing  elements  (PEs)  in  a program must call SHPCLMOVE with the
 *   same value of addr to maintain symmetric heap consistency; if  any  PEs
 *   are missing, the program hangs.
 */ 

void
FORTRANIFY(pshpclmove)(void *addr, size_t *length, long *errcode, int *abort)
{
  INIT_CHECK();

  addr = pshrealloc(addr, *length);

  /* pass back status code */
  *errcode = malloc_error;

  /* if malloc succeeded, nothing else to do */
  if (malloc_error == SHMEM_MALLOC_OK) {
    return;
    /* NOT REACHED */
  }

  /* failed somehow, we might have to abort */
  __shmem_trace(*abort ? SHMEM_LOG_FATAL : SHMEM_LOG_MEMORY,
		"shpdeallc() failed: %s",
		sherror()
		);
  /* MAYBE NOT REACHED */
}

#pragma weak shpalloc_ = pshpalloc_
#pragma weak shpdeallc_ = pshpdeallc_
#pragma weak shpclmove_ = pshpclmove_
