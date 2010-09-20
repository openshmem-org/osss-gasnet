#include "comms.h"

/*
 * assume yes, for now
 *
 * TODO: what kind of tests could we do to really check this?
 *       (maybe send a test ping-pong message)
 */
int
shmem_pe_accessible(int pe)
{
  return 1;
}

/*
 * only true if address has been allocated by shmem routine
 */
int
shmem_addr_accessible(void *addr, int pe)
{
  return __symmetric_var_in_range(addr, pe);
}
