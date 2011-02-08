#include "comms.h"

/*
 * assume yes, for now
 *
 * TODO: what kind of tests could we do to really check this?
 *       (maybe send a test ping-pong message)
 */

/* @api@ */
int
pshmem_pe_accessible(int pe)
{
  return 1;
}

/*
 * only true if address can be accessed through SHMEM
 *
 */

/* @api@ */
int
pshmem_addr_accessible(void *addr, int pe)
{
  return __comms_addr_accessible(addr, pe);
}

#pragma weak shmem_pe_accessible = pshmem_pe_accessible
#pragma weak shmem_addr_accessible = pshmem_addr_accessible
