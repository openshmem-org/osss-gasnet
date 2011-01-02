#include <stdio.h>
#include <string.h>

#include <mpp/shmem.h>

int pWrk[_SHMEM_REDUCE_SYNC_SIZE];
long pSync[_SHMEM_BCAST_SYNC_SIZE];

int src;
int dst;

int
main()
{
  memset(pSync, _SHMEM_BCAST_SYNC_SIZE, 0);

  start_pes(0);

  src = _my_pe() + 1;
  shmem_barrier_all();

  shmem_int_sum_to_all(&dst, &src, 1, 0, 0, 4, pWrk, pSync);

  printf("%d/%d   dst =", _my_pe(), _num_pes() );
  printf(" %d", dst);
  printf("\n");

  return 0;
}
