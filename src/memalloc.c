/*
 * Layer to sit on top of the malloc library, exposed to the shmem runtime.
 * This is PE-local.
 */

#include <sys/types.h>

#include "dlmalloc.h"

/* (c|sh)ould be static at some point */
mspace myspace;

/*
 * initialize the memory pool
 */
void
__mem_init(void *base, size_t capacity)
{
  myspace = create_mspace_with_base(base, capacity, 1);
}

/*
 * clean up memory pool
 */
void
__mem_finalize(void)
{
  destroy_mspace(myspace);
}

/*
 * return start of pool
 */
void *
__mem_base(void)
{
  return myspace;
}

void *
__mem_alloc(size_t size)
{
  return mspace_malloc(myspace, size);
}

void
__mem_free(void *addr)
{
  return mspace_free(myspace, addr);
}

void *
__mem_realloc(void *addr, size_t size)
{
  return mspace_realloc(myspace, addr, size);
}

void *
__mem_align(size_t alignment, size_t size)
{
  return mspace_memalign(myspace, alignment, size);
}
