#include <stdio.h>
#include <string.h>

#include <mpp/shmem.h>

int pWrk[_SHMEM_REDUCE_SYNC_SIZE];
long pSync[_SHMEM_BCAST_SYNC_SIZE];

int src[3];
int dst[3];

int
main()
{
  int i;

  memset(pSync, _SHMEM_BCAST_SYNC_SIZE, 0);

  start_pes(0);

  for (i = 0; i < 3; i += 1) {
    src[i] = _my_pe() + i;
  }
  shmem_barrier_all();

  shmem_int_sum_to_all(dst, src, 3, 0, 0, 4, pWrk, pSync);

  printf("%d/%d   dst =", _my_pe(), _num_pes() );
  for (i = 0; i < sizeof(dst)/sizeof(int); i+= 1) {
    printf(" %d", dst[i]);
  }
  printf("\n");

  return 0;
}
