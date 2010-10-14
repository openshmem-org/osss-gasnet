#include <stdio.h>
#include <assert.h>
#include <sys/types.h>

#include "state.h"
#include "comms.h"
#include "warn.h"
#include "shmem.h"

#include "memalloc.h"

long malloc_error = SHMEM_MALLOC_OK; /* exposed for error codes */

/*
 * TODO: shmalloc should check that ALL PEs got passed the same
 * "size", return NULL and set malloc_error if not.
 *
 * definitely need internal unchecked shmalloc layer for this
 */

static long *shmalloc_remote_size;

static int
shmalloc_symmetry_check(size_t size)
{
  int i = 0;
  int succeeded = 1;
  long shmalloc_received_size;

  *shmalloc_remote_size = size;
  shmem_barrier_all();
  for (; i < __state.numpes; i+= 1) {
    if (i == __state.mype) {
      continue;
    }
    shmalloc_received_size = shmem_long_g(shmalloc_remote_size, i);
    if (shmalloc_received_size != size) {
      __shmem_warn(SHMEM_LOG_NOTICE,
		   "shmalloc was given %ld on PE %d",
		   shmalloc_received_size, i);
      malloc_error = SHMEM_MALLOC_SYMMSIZE_FAILED;
      succeeded = 0;
      break;
    }
  }
  return succeeded;
}

void *
shmalloc(size_t size)
{
  void *addr;

  addr = __mem_alloc(size);

  if (addr == (void *) NULL) {
    __shmem_warn(SHMEM_LOG_NOTICE,
		 "shmalloc(%ld) failed",
		 size);
    malloc_error = SHMEM_MALLOC_FAIL;
  }
  else {
    malloc_error = SHMEM_MALLOC_OK;
  }

  shmem_barrier_all();		/* so say the SGI docs */

  return addr;
}

void
shfree(void *addr)
{
  if (addr == (void *) NULL) {
    __shmem_warn(SHMEM_LOG_NOTICE,
		 "address passed to shfree() already null");
    malloc_error = SHMEM_MALLOC_ALREADY_FREE;
    return;
  }

  __shmem_warn(SHMEM_LOG_DEBUG, "shfree(%p) in pool @ %p", addr, __mem_base());

  __mem_free(addr);

  malloc_error = SHMEM_MALLOC_OK;

  shmem_barrier_all();
}

void *
shrealloc(void *addr, size_t size)
{
  void *newaddr;

  if (addr == (void *) NULL) {
    __shmem_warn(SHMEM_LOG_NOTICE,
		 "address passed to shrealloc() already null");
    malloc_error = SHMEM_MALLOC_ALREADY_FREE;
    return (void *) NULL;
  }

  newaddr = __mem_realloc(addr, size);

  if (newaddr == (void *) NULL) {
    __shmem_warn(SHMEM_LOG_NOTICE,
		 "shrealloc() couldn't reallocate %ld bytes @ %p",
		 size, addr);
    malloc_error = SHMEM_MALLOC_REALLOC_FAILED;
  }
  else {
    malloc_error = SHMEM_MALLOC_OK;
  }

  shmem_barrier_all();

  return newaddr;
}

/*
 * The shmemalign function allocates a block in the symmetric heap that
 * has a byte alignment specified by the alignment argument.
 */
void *
shmemalign(size_t alignment, size_t size)
{
  void *addr = __mem_align(alignment, size);

  if (addr == (void *) NULL) {
    __shmem_warn(SHMEM_LOG_NOTICE,
		 "shmem_memalign() couldn't resize %ld bytes to alignment %ld",
		 size, alignment);
    malloc_error = SHMEM_MALLOC_MEMALIGN_FAILED;
  }
  else {
    malloc_error = SHMEM_MALLOC_OK;
  }

  shmem_barrier_all();

  return addr;
}
