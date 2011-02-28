/*
 * Layer to sit on top of the malloc library, exposed to the shmem runtime.
 * This is PE-local and sits just below SHMEM itself.
 */

#include <sys/types.h>

#include "dlmalloc.h"

/*
 * the memory area we manage in this unit.  Not visible to anyone else
 */
static mspace myspace;

/*
 * initialize the memory pool
 */
void
__shmem_mem_init(void *base, size_t capacity)
{
  myspace = create_mspace_with_base(base, capacity, 1);
}

/*
 * clean up memory pool
 */
void
__shmem_mem_finalize(void)
{
  destroy_mspace(myspace);
}

/*
 * return start of pool
 */
void *
__shmem_mem_base(void)
{
  return myspace;
}

void *
__shmem_mem_alloc(size_t size)
{
  return mspace_malloc(myspace, size);
}

void
__shmem_mem_free(void *addr)
{
  return mspace_free(myspace, addr);
}

void *
__shmem_mem_realloc(void *addr, size_t size)
{
  return mspace_realloc(myspace, addr, size);
}

void *
__shmem_mem_align(size_t alignment, size_t size)
{
  return mspace_memalign(myspace, alignment, size);
}
