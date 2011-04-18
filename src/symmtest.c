#include <stdio.h>               /* NULL                           */

#include "trace.h"

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
