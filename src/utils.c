#include "state.h"
#include "trace.h"
#include "utils.h"

/*
 * check the target PE "pe" is within the program range
 *
 */

void
__shmem_pe_range_check(int pe)
{
  const int bot_pe = 0;
  const int top_pe = GET_STATE(numpes) - 1;

  if (pe >= bot_pe && pe <= top_pe) {
    /* PE is within program range */
    return;
    /* NOT REACHED */
  }

  __shmem_trace(SHMEM_LOG_FATAL,
                "Target PE %d not within allocated range %d .. %d",
                pe, bot_pe, top_pe
		);
  /* NOT REACHED */
}
