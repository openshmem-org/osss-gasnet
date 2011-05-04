/*
 * reduce by OR() [1,2,3,4,...n] across all PEs, giving 2n - 1 (bit-wise)
 *
 */

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
  int i;
  int me;
  int npes;

  for (i = 0; i < SHMEM_BCAST_SYNC_SIZE; i += 1) {
    pSync[i]  =_SHMEM_SYNC_VALUE;
  }

  start_pes(0);
  me = _my_pe();
  npes = _num_pes();

  src = me + 1;
  shmem_barrier_all();

  shmem_int_or_to_all(&dst, &src, 1, 0, 0, npes, pWrk, pSync);

  printf("%d/%d   dst = %d\n", me, npes, dst);

  return 0;
}
