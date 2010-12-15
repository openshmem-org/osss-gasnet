#include <stdio.h>
#include <mpp/shmem.h>

int
main()
{
  short source[10] = { 1, 2, 3, 4, 5,
                       6, 7, 8, 9, 10 };
  static short target[10];
  int i;
  int me;

  for (i = 0; i < 10; i+= 1) {
    target[i] = 666;
  }

  start_pes(2);
  me = _my_pe();

  if (me == 0) {
    shmem_short_iput(target, source, 2, 1, 4, 1);
  }
  shmem_barrier_all();   /* sync sender and receiver */
  if (me == 1) {
    for (i = 0; i < 10; i+= 1) {
      printf("PE %d: target[%d] = %hd\n", me, i, target[i]);
    }
  }
  shmem_barrier_all();   /* sync before exiting */

  return 0;
}
