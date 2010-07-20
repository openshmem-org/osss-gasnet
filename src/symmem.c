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
  return (seginfo_table[pe].addr <= addr) && (addr < top) ? 1 : 0;
}

/*
 * PUBLIC INTERFACE
 */

long malloc_error = 0;         /* exposed for error codes */

static const size_t MIN_MALLOC_EX_SIZE = 32 * 8;

void *
shmalloc(size_t size)
{
  void *pool;
  void *addr;

  pool = __symmetric_var_base(__state.mype);

  /* TODO: bizarre issue with 23 or 24 longs as first malloc */
  if (size < MIN_MALLOC_EX_SIZE) {
    size = MIN_MALLOC_EX_SIZE;
  }

  addr = malloc_ex(size, pool);

  if (addr == (void *)NULL) {
    __shmem_warn("NOTICE", "shmalloc(%ld) failed\n", size);
    malloc_error = SHMEM_MALLOC_FAIL;
  }

  return addr;
}

void
shfree(void *addr)
{
  void *pool;

  if (addr == (void *)NULL) {
    __shmem_warn("NOTICE", "shfree(%p) already null\n", addr);
    malloc_error = SHMEM_MALLOC_ALREADY_FREE;
    return;
  }

  pool = __symmetric_var_base(__state.mype);

  free_ex(addr, pool);

  malloc_error = 0;
}

void *
shrealloc(void *addr, size_t size)
{
  void *pool = __symmetric_var_base(__state.mype);
  void *newaddr;

  if (addr == (void *)NULL) {
    __shmem_warn("NOTICE", "shrealloc(%p) null\n", addr);
    malloc_error = SHMEM_MALLOC_ALREADY_FREE;
    return (void *)NULL;
  }

  newaddr = realloc_ex(addr, size, pool);

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
  return shmalloc(size);
}
