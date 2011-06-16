/* (c) 2011 University of Houston.  All rights reserved. */


#include "state.h"
#include "utils.h"

/*
 * initialize the PE's state (this is all we need to initialize)
 */

state_t __state = {
  .pe_status = PE_UNINITIALIZED
};

/*
 * PE status and its human description
 */

struct state_desc {
  pe_status_t s;
  const char *desc;
};

/*
 * table of known PE status
 */

static struct state_desc d[] =
  {
    { PE_UNINITIALIZED, "PE has not been initialized yet" },
    { PE_UNKNOWN,       "I have no information about PE" },
    { PE_RUNNING,       "PE is running" },
    { PE_SHUTDOWN,      "PE has been cleanly shut down" },
    { PE_FAILED,        "PE has failed" },
  };
static const int nd = TABLE_SIZE(d);

/*
 * translate PE status to human description
 */

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
