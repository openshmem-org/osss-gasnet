/* (c) 2011 University of Houston System.  All rights reserved. */


/*
 * just do the cache calls.   No output expected
 *
 */

#include <stdio.h>

#include <mpp/shmem.h>

long var;

int
main(void)
{
  start_pes(0);

  shmem_clear_cache_inv();

  shmem_set_cache_inv();

  shmem_clear_cache_line_inv(&var);

  shmem_set_cache_line_inv(&var);

  shmem_udcflush();

  shmem_udcflush_line(&var);

  return 0;
}
