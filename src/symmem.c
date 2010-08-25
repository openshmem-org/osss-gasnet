#include <stdio.h>
#include <assert.h>
#include <sys/types.h>

#include "state.h"
#include "comms.h"
#include "warn.h"
#include "shmem.h"

#include "dlmalloc.h"

/*
 * Tony's development interface
 */

mspace myspace;

/*
 * PUBLIC INTERFACE
 */

long malloc_error = SHMEM_MALLOC_OK; /* exposed for error codes */

void *
shmalloc(size_t size)
{
  void *addr;

  addr = mspace_malloc(myspace, size);

  if (addr == (void *) NULL) {
    __shmem_warn(SHMEM_LOG_NOTICE, "shmalloc(%ld) failed", size);
    malloc_error = SHMEM_MALLOC_FAIL;
  }
  else {
    malloc_error = SHMEM_MALLOC_OK;
  }

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

  __shmem_warn(SHMEM_LOG_DEBUG, "shfree(%p) in pool @ %p", addr, myspace);

  mspace_free(myspace, addr);

  malloc_error = SHMEM_MALLOC_OK;
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

  newaddr = mspace_realloc(myspace, addr, size);

  if (newaddr == (void *) NULL) {
    __shmem_warn(SHMEM_LOG_NOTICE,
		 "shrealloc() couldn't reallocate %ld bytes @ %p",
		 size, addr);
    malloc_error = SHMEM_MALLOC_REALLOC_FAILED;
  }
  else {
    malloc_error = SHMEM_MALLOC_OK;
  }

  return newaddr;
}

/*
 * The shmemalign function allocates a block in the symmetric heap that
 * has a byte alignment specified by the alignment argument.
 */
void *
shmemalign(size_t alignment, size_t size)
{
  void *addr = mspace_memalign(myspace, alignment, size);

  if (addr == (void *) NULL) {
    __shmem_warn(SHMEM_LOG_NOTICE,
		 "shmem_memalign() couldn't resize %ld bytes to alignment %ld",
		 size, alignment);
    malloc_error = SHMEM_MALLOC_MEMALIGN_FAILED;
  }
  else {
    malloc_error = SHMEM_MALLOC_OK;
  }

  return addr;
}
