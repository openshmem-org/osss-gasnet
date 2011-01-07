#include <stdio.h>

#include <mpp/shmem.h>

#define NPES 4

long pSync[NPES];
int x = 10101;

int
main()
{
  int me, npes;
  int i;

  for (i = 0; i < NPES; i += 1) {
    pSync[i] = _SHMEM_SYNC_VALUE;
  }

  start_pes(0);
  me = _my_pe();
  npes = _num_pes();

  if (me == 0) {
    shmem_int_p(&x, 4, 1);
  }

  shmem_barrier(0, 0, npes, pSync);

  printf("%d: x = %d\n", me, x);

  return 0;
}
