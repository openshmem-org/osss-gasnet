/*
 * rotate PE id to right neighbor (dest), with wrap-around
 */

#include <stdio.h>
#include <time.h>
#include <assert.h>

#include <mpp/shmem.h>

int
main(int argc, char **argv)
{
  int nextpe;
  int me, npes;
  int src;
  int *dest;

  start_pes(0);
  me = _my_pe();
  npes = _num_pes();

  nextpe = (me + 1) % npes;

  src = nextpe;

  dest = (int *) shmalloc(sizeof(*dest));
  assert(dest != NULL);

  *dest = -1;
  shmem_barrier_all();

  shmem_int_put(dest, &src, 1, nextpe);

  shmem_barrier_all();

  printf("%4d: got %4d: ", me, *dest);
  if (*dest == me) {
    printf("CORRECT");
  }
  else {
    printf("WRONG, expected %d", me);
  }
  printf("\n");

  shfree(dest);

  return 0;
}
