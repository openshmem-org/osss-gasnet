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

static void *my_pool;

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

  my_pool = seginfo_table[__state.mype].addr;

  {
    size_t ps = init_memory_pool(seginfo_table[__state.mype].size,
				 my_pool);
    __shmem_warn(SHMEM_LOG_DEBUG, "pool size is %ld", ps);
  }

  __gasnet_barrier_all();
}

void
__symmetric_memory_finalize(void)
{
   destroy_memory_pool(my_pool);
}

/*
 * where the symmetric memory starts on the given PE
 */
__inline__
void *
__symmetric_var_base(int pe)
{
#define ROUNDUP8(v) ((v + 7) & ~7)

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

static const size_t MIN_MALLOC_EX_SIZE = 32 * sizeof(long);
static int first_malloc = 1;

void *
shmalloc(size_t size)
{
  void *addr;

  /* TODO: bizarre issue with 23 or 24 longs as first malloc */
  if (first_malloc && (size == 184 || size == 192)) {
    // __shmem_warn(SHMEM_LOG_NOTICE, "first shmalloc(%ld) rounding up to %ld", size, MIN_MALLOC_EX_SIZE);
    size = MIN_MALLOC_EX_SIZE;
  }
  first_malloc = 0;

  addr = malloc_ex(size, my_pool);

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

  __shmem_warn(SHMEM_LOG_DEBUG, "shfree(%p) does nothing right now", addr);

  free_ex(addr, my_pool);

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

  newaddr = realloc_ex(addr, size, seginfo_table[__state.mype].addr);

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
