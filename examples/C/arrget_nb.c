
/*
 * rotate get_nb an array to right neighbor
 */

#include <stdio.h>
#include <time.h>

#include <shmem.h>

#define N 7

int
main (int argc, char **argv)
{
  int i;
  int nextpe;
  int me, npes;
  long src[N];
  long *dest;
  void *handle;

  start_pes (0);
  me = _my_pe ();
  npes = _num_pes ();

  for (i = 0; i < N; i += 1)
    {
      src[i] = (long) me;
    }

  dest = (long *) shmalloc (N * sizeof (*dest));

  nextpe = (me + 1) % npes;

  handle = shmem_long_get_nb (dest, src, N, nextpe, &handle);
  shmem_wait_nb(handle);

  shmem_barrier_all ();

  shfree (dest);

  return 0;
}
