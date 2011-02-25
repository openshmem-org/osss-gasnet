#include "utils.h"

#include "shmem.h"

void
pshmem_version(int *major, int *minor)
{
  *major = SHMEM_MAJOR_VERSION;
  *minor = SHMEM_MINOR_VERSION;
}

#pragma weak shmem_version = pshmem_version
