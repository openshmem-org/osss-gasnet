#include "state.h"
#include "utils.h"

state_t __state = {
  .pe_status = PE_UNINITIALIZED /* other fields = don't care yet */
};

struct state_desc {
  pe_status_t s;
  const char *desc;
};

static struct state_desc d[] =
  {
    { PE_UNINITIALIZED, "PE has not been initialized yet" },
    { PE_UNKNOWN,       "I have no information about PE" },
    { PE_RUNNING,       "PE is already running" },
    { PE_SHUTDOWN,      "PE has been cleanly shut down" },
    { PE_FAILED,        "PE has failed" },
  };
static const int nd = TABLE_SIZE(d);

const char *
__shmem_state_as_string(pe_status_t s)
{
  struct state_desc *dp = d;
  int i;

  for (i = 0; i < nd; i += 1) {
    if (s == dp->s) {
      return dp->desc;
      /* NOT REACHED */
    }
    dp += 1;
  }

  return "unknown state";
}
