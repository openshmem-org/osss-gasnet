/*
 * run on 4 PEs
 *
 */

#include <stdio.h>
#include <string.h>

#include <mpp/shmem.h>

long pSync[_SHMEM_BCAST_SYNC_SIZE];

int
main(void)
{
  int npes;
  int me;
  int *dst;
  static int src;
  int i;

  start_pes(0);
  npes = _num_pes();
  me = _my_pe();

  dst = shmalloc(sizeof(*dst) * 4);

  for (i = 0; i < 4; i++) {
    dst[i] = 10101;
  }
  src = me + 100;

  for (i = 0; i < _SHMEM_BCAST_SYNC_SIZE; i += 1) {
    pSync[i] = _SHMEM_SYNC_VALUE;
  }
  shmem_barrier_all();

  printf("%8s: dst[%d/%d] = %d, %d, %d, %d\n",
         "BEFORE", me, npes, dst[0], dst[1], dst[2], dst[3]);

  shmem_fcollect32(dst, &src, 1,
                   0, 0, npes,
                   pSync);

  shmem_barrier_all();

  /*
   * end state of "dst" = 100, 101, 102, ...
   */

  printf("%8s: dst[%d/%d] = %d, %d, %d, %d\n",
         "AFTER", me, npes, dst[0], dst[1], dst[2], dst[3]);

  shfree(dst);

  return 0;
}
