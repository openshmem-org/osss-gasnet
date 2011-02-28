#include "utils.h"

#include "shmem.h"

/*
 * OpenSHMEM has a major.minor release number.  Return 0 if
 * successful, -1 otherwise
 *
 */

/* @api@ */
int
pshmem_version(int *major, int *minor)
{
#if ! defined(SHMEM_MAJOR_VERSION) && ! defined(SHMEM_MINOR_VERSION)
  return -1;
#endif /* ! SHMEM_MAJOR_VERSION && ! SHMEM_MINOR_VERSION */
  *major = SHMEM_MAJOR_VERSION;
  *minor = SHMEM_MINOR_VERSION;
  return 0;
}

#pragma weak shmem_version = pshmem_version
