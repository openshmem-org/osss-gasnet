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
 * check that all PEs see the same shmalloc size: return first
 * mis-matching PE id if there's a mis-match, return -1 to record
 * correct symmetry
 */

static int
shmalloc_symmetry_check(size_t size)
{
  int pe;
  int any_failed_pe = -1;
  long shmalloc_received_size;
  long *shmalloc_remote_size;

  /* record for everyone else to see */
  shmalloc_remote_size = (long *) __mem_alloc(sizeof(*shmalloc_remote_size));
  if (shmalloc_remote_size == (long *) NULL) {
    __shmem_warn(SHMEM_LOG_FATAL,
		 "internal error: couldn't allocate memory for symmetry check"
		 );
    /* NOT REACHED */
  }
  *shmalloc_remote_size = size;
  shmem_barrier_all();

  /* everyone checks everyone else's sizes, barf if mis-match */
  for (pe = 0; pe < __state.numpes; pe += 1) {
    if (pe == __state.mype) {
      continue;
    }
    shmalloc_received_size = shmem_long_g(shmalloc_remote_size, pe);
    if (shmalloc_received_size != size) {
      __shmem_warn(SHMEM_LOG_NOTICE,
		   "shmalloc expected %ld, but saw %ld on PE %d",
		   size, shmalloc_received_size, pe
		   );
      malloc_error = SHMEM_MALLOC_SYMMSIZE_FAILED;
      any_failed_pe = pe;
      break;
    }
  }
  __mem_free(shmalloc_remote_size);
  return any_failed_pe;
}

/* @api@ */
void *
shmalloc(size_t size)
{
  void *addr;

  if (shmalloc_symmetry_check(size) != -1) {
    malloc_error = SHMEM_MALLOC_SYMMSIZE_FAILED;
    return (void *) NULL;
  }

  __shmem_warn(SHMEM_LOG_MEMORY,
	       "shmalloc(%ld bytes) passed symmetry check",
	       size
	       );

  addr = __mem_alloc(size);

  if (addr == (void *) NULL) {
    __shmem_warn(SHMEM_LOG_NOTICE,
		 "shmalloc(%ld bytes) failed",
		 size
		 );
    malloc_error = SHMEM_MALLOC_FAIL;
  }
  else {
    malloc_error = SHMEM_MALLOC_OK;
  }

  __shmem_warn(SHMEM_LOG_MEMORY,
	       "shmalloc(%ld bytes) @ %p",
	       size, addr
	       );

  shmem_barrier_all();		/* so say the SGI docs */

  return addr;
}
#pragma weak shmem_malloc = shmalloc

/* @api@ */
void
shfree(void *addr)
{
  if (addr == (void *) NULL) {
    __shmem_warn(SHMEM_LOG_MEMORY,
		 "address passed to shfree() already null"
		 );
    malloc_error = SHMEM_MALLOC_ALREADY_FREE;
    return;
  }

  __shmem_warn(SHMEM_LOG_MEMORY,
	       "shfree(%p) in pool @ %p",
	       addr, __mem_base()
	       );

  __mem_free(addr);

  malloc_error = SHMEM_MALLOC_OK;

  shmem_barrier_all();
}
#pragma weak shmem_free = shfree

/* @api@ */
void *
shrealloc(void *addr, size_t size)
{
  void *newaddr;

  if (shmalloc_symmetry_check(size) != -1) {
    malloc_error = SHMEM_MALLOC_SYMMSIZE_FAILED;
    return (void *) NULL;
  }

  if (addr == (void *) NULL) {
    __shmem_warn(SHMEM_LOG_MEMORY,
		 "address passed to shrealloc() already null"
		 );
    malloc_error = SHMEM_MALLOC_ALREADY_FREE;
    return (void *) NULL;
  }

  newaddr = __mem_realloc(addr, size);

  if (newaddr == (void *) NULL) {
    __shmem_warn(SHMEM_LOG_MEMORY,
		 "shrealloc(%ld bytes) failed @ original address %p",
		 size, addr
		 );
    malloc_error = SHMEM_MALLOC_REALLOC_FAILED;
  }
  else {
    malloc_error = SHMEM_MALLOC_OK;
  }

  shmem_barrier_all();

  return newaddr;
}
#pragma weak shmem_realloc = shrealloc

/*
 * The shmemalign function allocates a block in the symmetric heap that
 * has a byte alignment specified by the alignment argument.
 */
/* @api@ */
void *
shmemalign(size_t alignment, size_t size)
{
  void *addr;

  if (shmalloc_symmetry_check(size) != -1) {
    malloc_error = SHMEM_MALLOC_SYMMSIZE_FAILED;
    return (void *) NULL;
  }

  addr = __mem_align(alignment, size);

  if (addr == (void *) NULL) {
    __shmem_warn(SHMEM_LOG_MEMORY,
		 "shmem_memalign(%ld bytes) couldn't realign to %ld",
		 size, alignment
		 );
    malloc_error = SHMEM_MALLOC_MEMALIGN_FAILED;
  }
  else {
    malloc_error = SHMEM_MALLOC_OK;
  }

  shmem_barrier_all();

  return addr;
}
#pragma weak shmem_memalign = shmemalign

/*
 * readable error message for error code "e"
 */

typedef struct {
  long code;
  char *msg;
} malloc_error_code_t;

static
malloc_error_code_t error_table[] =
  {
    { SHMEM_MALLOC_OK,
      "no symmetric memory allocation error"                  },
    { SHMEM_MALLOC_FAIL,
      "symmetric memory allocation failed"                    },
    { SHMEM_MALLOC_ALREADY_FREE,
      "attempt to free already null symmetric memory address" },
    { SHMEM_MALLOC_MEMALIGN_FAILED,
      "attempt to align symmetric memory address failed"      },
    { SHMEM_MALLOC_REALLOC_FAILED,
      "attempt to reallocate symmetric memory address failed" },
    { SHMEM_MALLOC_SYMMSIZE_FAILED,
      "asymmetric sizes passed to symmetric memory allocator" }
  };
static const int nerrors = sizeof(error_table) / sizeof(error_table[0]);

/* @api@ */
char *
sherror(void)
{
  malloc_error_code_t *etp = error_table;
  int i;

  for (i = 0; i < nerrors; i+= 1) {
    if (malloc_error == etp->code) {
      return etp->msg;
    }
    etp += 1;
  }

  return "unknown error";
}
#pragma weak shmem_error = sherror

#ifdef HAVE_PSHMEM_SUPPORT
#pragma weak pshmalloc = shmalloc
#pragma weak pshmem_malloc = shmem_malloc
#pragma weak pshfree = shfree
#pragma weak pshmem_free = shmem_free
#pragma weak pshrealloc = shrealloc
#pragma weak pshmem_realloc = shmem_realloc
#pragma weak pshmemalign = shmemalign
#pragma weak pshmem_memalignloc = shmem_memalign
#endif /* HAVE_PSHMEM_SUPPORT */
