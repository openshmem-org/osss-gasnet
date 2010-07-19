/*
 * TODO: shmem_ptr only makes sense on platforms where puts and gets
 * occur exclusively in shared-memory.  On a multi-node cluster, it
 * can't do anything, so return NULL, which is correct behavior.
 * (which is really good, because I couldn't see how it could possibly
 * work :-)
 */

#include <stdio.h>

#include "state.h"
#include "warn.h"

void *
shmem_ptr(void *target, int pe)
{
#ifdef SHMEM_PUTGET_SHARED_MEMORY

  __shmem_warn("NOTICE", shmem_ptr() not implemented yet");
  return (void *)NULL;

#else  /* ! SHMEM_PUTGET_SHARED_MEMORY */

  return (void *)NULL;

#endif /* SHMEM_PUTGET_SHARED_MEMORY*/
}
