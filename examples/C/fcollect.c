#include <stdio.h>
#include <string.h>

#include <assert.h>

#include <shmem.h>

int
main(void)
{
  int npes;
  int me;
  int *dst, *src;
  int i;
  long *pSync;

  shmem_init();
  npes = shmem_num_pes();
  me = shmem_my_pe();

  dst = shmalloc(sizeof(*dst) * 4);
  src = shmalloc(sizeof(*src));

  for (i = 0; i < 4; i++) {
    dst[i] = 10101;
  }
  *src = me + 100;

  shmem_barrier_all();

  pSync = shmem_sync_init();

  printf("%8s: dst[%d/%d] = %d, %d, %d, %d\n",
         "BEFORE", me, npes, dst[0], dst[1], dst[2], dst[3]);

  shmem_fcollect32(dst, src, 1,
                   0, 1, 4,
                   pSync);

  shmem_barrier_all();

  /*
   * state of "dst", indices 0, 2, 4, 6 should be "src" values
   */

  printf("%8s: dst[%d/%d] = %d, %d, %d, %d\n",
         "AFTER", me, npes, dst[0], dst[1], dst[2], dst[3]);

  shfree(dst);
  shfree(src);

  return 0;
}
