/* (c) 2011 University of Houston System.  All rights reserved. */


#include <stdio.h>               /* NULL                           */

#include "trace.h"
#include "globalvar.h"
#include "comms.h"
#include "state.h"

/*
 * Deprecated
 *
 */

void
__shmem_symmetric_test_with_abort(void *remote_addr,
				  void *local_addr,
				  const char *name,
				  const char *routine)
{
  if (remote_addr == NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "shmem_%s_%s: address %p is not symmetric",
		  name, routine,
		  local_addr
		  );
    /* NOT REACHED */
  }
}

/*
 * check that the address is accessible to shmem on that PE
 *
 */
int
__shmem_symmetric_addr_accessible(void *addr, int pe)
{
  return (__shmem_symmetric_addr_lookup(addr, pe) != NULL);
}

int
__shmem_is_symmetric(void *addr)
{
  return
    __shmem_symmetric_is_globalvar(addr)
    ||
    __shmem_symmetric_var_in_range(addr, GET_STATE(mype));
}
