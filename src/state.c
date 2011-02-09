#include "state.h"

state_t __state = {
  .pe_status = PE_UNINITIALIZED /* other fields = don't care yet */
};

char *
__shmem_state_as_string(pe_status_t s)
{
  if (s == PE_UNINITIALIZED) {
    return "PE has not been initialized yet";
  }
  if (s == PE_RUNNING) {
    return "PE is already running";
  }
  if (s == PE_SHUTDOWN) {
    return "PE has been cleanly shut down";
  }
  if (s == PE_FAILED) {
    return "PE has failed";
  }

  return "unknown state";
}
