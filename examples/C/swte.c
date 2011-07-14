/* (c) 2011 University of Houston System.  All rights reserved. */


/*
 * conditional swap-to-self test (for local in-memory writes)
 *
 *
 */

#include <stdio.h>
#include <mpp/shmem.h>

static int value;

int
main()
{
  int old;
  int me;

  start_pes(0);
  me = _my_pe();

  value = me + 1;

  old = shmem_int_cswap(&value, value, -value, me);

  printf("%d: value = %d, old = %d\n", me, value, old);

  return 0;
}
