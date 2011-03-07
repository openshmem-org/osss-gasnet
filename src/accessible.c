#include "comms.h"
#include "utils.h"

/*
 * assume yes, for now
 *
 */

/* @api@ */
int
pshmem_pe_accessible(int pe)
{
  INIT_CHECK();
  return __shmem_comms_ping_request(pe);
}

/*
 * only true if address can be accessed through SHMEM
 *
 */

/* @api@ */
int
pshmem_addr_accessible(void *addr, int pe)
{
  INIT_CHECK();
  return __shmem_comms_addr_accessible(addr, pe);
}

#pragma weak shmem_pe_accessible = pshmem_pe_accessible
#pragma weak shmem_addr_accessible = pshmem_addr_accessible
