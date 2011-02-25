#include <stdio.h>

#include <mpp/shmem.h>

int
main()
{
  static int race_winner = -1;
  int oldval;
  int me;

  start_pes(2);
  me = _my_pe();

  oldval = shmem_int_cswap(&race_winner, -1, me, 0);

  if (oldval == -1) {
    printf("pe %d was first\n", me);
  }

  return 0;
}
