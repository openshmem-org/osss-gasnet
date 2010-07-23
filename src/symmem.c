#include <assert.h>
#include <sys/types.h>

#include "gasnet_safe.h"


#include "state.h"
#include "barrier.h"
#include "warn.h"

#include "shmem.h"

#include "dlmalloc.h"

/*
 * Tony's development interface
 */

static gasnet_seginfo_t* seginfo_table;

static mspace myspace;

#define ROUNDUP(v, n) (((v) + (n)) & ~((n) - 1))

void
__symmetric_memory_init(void)
{
  seginfo_table = (gasnet_seginfo_t *)calloc(__state.numpes,
                                             sizeof(gasnet_seginfo_t));
  assert(seginfo_table != (gasnet_seginfo_t *)NULL);

  GASNET_SAFE( gasnet_getSegmentInfo(seginfo_table, __state.numpes) );

  /*
   * each PE initializes its own table, but can see addresses of all PEs
   */

  myspace = create_mspace_with_base(seginfo_table[__state.mype].addr,
				    seginfo_table[__state.mype].size,
				    1);

  __gasnet_barrier_all();
}

void
__symmetric_memory_finalize(void)
{
  destroy_mspace(myspace);
}

/*
 * where the symmetric memory starts on the given PE
 */
__inline__
void *
__symmetric_var_base(int pe)
{
  return seginfo_table[pe].addr;
}

/*
 * is the address in the managed symmetric area?
 */
__inline__
int
__symmetric_var_in_range(void *addr, int pe)
{
  void *top = seginfo_table[pe].addr + seginfo_table[pe].size;
  return (seginfo_table[pe].addr <= addr) && (addr <= top) ? 1 : 0;
}

/*
 * PUBLIC INTERFACE
 */

long malloc_error = 0;         /* exposed for error codes */

void *
shmalloc(size_t size)
{
  void *addr;

  addr = mspace_malloc(myspace, size);

  if (addr == (void *)NULL) {
    __shmem_warn(SHMEM_LOG_NOTICE, "shmalloc(%ld) failed", size);
    malloc_error = SHMEM_MALLOC_FAIL;
  }

  return addr;
}

void
shfree(void *addr)
{
  if (addr == (void *)NULL) {
    __shmem_warn(SHMEM_LOG_NOTICE, "address passed to shfree already null");
    malloc_error = SHMEM_MALLOC_ALREADY_FREE;
    return;
  }

  __shmem_warn(SHMEM_LOG_DEBUG, "shfree(%p) in pool @ %p", addr, myspace);

  mspace_free(myspace, addr);

  malloc_error = 0;
}

void *
shrealloc(void *addr, size_t size)
{
  void *newaddr;

  if (addr == (void *)NULL) {
    __shmem_warn(SHMEM_LOG_NOTICE, "address passed to shrealloc already null");
    malloc_error = SHMEM_MALLOC_ALREADY_FREE;
    return (void *)NULL;
  }

  newaddr = mspace_realloc(myspace, addr, size);

  malloc_error = 0;

  return newaddr;
}

/*
 * TODO: shmemalign, this is probably wrong
 *
 * The shmemalign function allocates a block in the symmetric heap that
 * has a byte alignment specified by the alignment argument.
 */
void *
shmemalign(size_t alignment, size_t size)
{
  return mspace_memalign(myspace, alignment, size);
}
