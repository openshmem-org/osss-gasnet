#include <assert.h>
#include <sys/types.h>

#include "gasnet_safe.h"

#include "tlsf.h"                /* symmetric var allocation       */

#include "state.h"
#include "barrier.h"
#include "warn.h"

#include "shmem.h"

/*
 * Tony's development interface
 */

static gasnet_seginfo_t* seginfo_table;

void
__symmetric_memory_init(void)
{
  seginfo_table = (gasnet_seginfo_t *)calloc(__state.numpes,
                                             sizeof(gasnet_seginfo_t));
  assert(seginfo_table != (gasnet_seginfo_t *)NULL);

  GASNET_SAFE( gasnet_getSegmentInfo(seginfo_table, __state.numpes) );

  __gasnet_barrier_all();

  /*
   * each PE initializes its own table, but can see addresses of all PEs
   */
  init_memory_pool(seginfo_table[__state.mype].size,
                   seginfo_table[__state.mype].addr);
  __gasnet_barrier_all();
}

/*
 * where the symmetric memory starts on the given PE
 */
void *
__symmetric_var_base(int pe)
{
  return seginfo_table[pe].addr;
}

/*
 * is the address in the managed symmetric area?
 */
int
__symmetric_var_in_range(void *addr, int pe)
{
  void *top = seginfo_table[pe].addr + seginfo_table[pe].size;
  return (seginfo_table[pe].addr <= addr) && (addr < top) ? 1 : 0;
}

/*
 * PUBLIC INTERFACE
 */

long malloc_error = 0;         /* exposed for error codes */

const long SHMEM_MALLOC_OK=0L;
const long SHMEM_MALLOC_FAIL=1L;
const long SHMEM_MALLOC_ALREADY_FREE=2L;

void *
shmalloc(size_t size)
{
  void *area = __symmetric_var_base(__state.mype);
  void *addr;

  addr = malloc_ex(size, area);

  if (addr == (void *)NULL) {
    __shmem_warn("NOTICE", "shmalloc(%ld) failed\n", size);
    malloc_error = SHMEM_MALLOC_FAIL;
  }

__shmem_warn("INFO", "allocating %ld @ %p", size, addr);

  return addr;
}

void
shfree(void *addr)
{
  void *area = __symmetric_var_base(__state.mype);

  if (addr == (void *)NULL) {
    __shmem_warn("NOTICE", "shfree(%p) already null\n", addr);
    malloc_error = SHMEM_MALLOC_ALREADY_FREE;
    return;
  }

__shmem_warn("INFO", "freeing @ %p", addr);

  // TODO: sometimes causes segfaults...
  free_ex(addr, area);

  malloc_error = 0;
}

/*
 * TODO: shrealloc
 */
void *
shrealloc(void *ptr, size_t size)
{
  return (void *)NULL;
}

/*
 * TODO: shmemalign
 */
void *
shmemalign(size_t alignment, size_t size)
{
  return (void *)NULL;
}
