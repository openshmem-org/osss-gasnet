#include "comms.h"

/*
 * assume yes, for now
 *
 * TODO: what kind of tests could we do to really check this?
 *       (maybe send a test ping-pong message)
 */

/* @api@ */
int
shmem_pe_accessible(int pe)
{
  return 1;
}

/*
 * only true if address can be accessed through SHMEM
 *
 */

/* @api@ */
int
shmem_addr_accessible(void *addr, int pe)
{
  /* explicitly managed? */
  if (__symmetric_var_in_range(addr, pe)) {
    return 1;
  }
  /* global, visible symbol? */
  if (__comms_is_globalvar(addr)) {
    return 1;
  }
  /* otherwise, no */
  return 0;
}


#ifdef HAVE_PSHMEM_SUPPORT
#pragma weak pshmem_pe_accessible = shmem_pe_accessible
#pragma weak pshmem_addr_accessible = shmem_addr_accessible
#endif /* HAVE_PSHMEM_SUPPORT */
