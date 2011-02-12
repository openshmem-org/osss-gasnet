#include "state.h"
#include "trace.h"
#include "comms.h"

static const char *ping_timeout_envvar = "SHMEM_PE_ACCESSIBLE_TIMEOUT";
static const int DEFAULT_PE_ACCESSIBLE_TIMEOUT = 1;

void
__shmem_ping_init(void)
{
  int timeout;
  char *pt = __comms_getenv(ping_timeout_envvar);

  if (pt != (char *) NULL) {
    timeout = atoi(pt);
  }
  else {
    timeout = DEFAULT_PE_ACCESSIBLE_TIMEOUT;
  }
  /* sanity check */
  if (timeout < 0) {
    timeout = DEFAULT_PE_ACCESSIBLE_TIMEOUT;
  }      

  __state.ping_timeout = timeout;

  __shmem_trace(SHMEM_LOG_INIT,
		"PE accessibility timeout set to %d sec",
		timeout
		);
}
