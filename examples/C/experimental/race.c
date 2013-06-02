#include <stdio.h>

#include <shmem.h>

int src = 42;

int
main()
{
  int *dst;
  int me;

  start_pes (0);
  me = _my_pe ();

  dst = shmalloc (sizeof (*dst));
  *dst = -999;
  shmem_barrier_all ();

  if (me == 0)
    {
      int s;
      void *h;

      h = shmem_int_put_nb (dst, &src, 1, 1, NULL);

      s = shmem_test_nb (h);

      fprintf (stderr, "%d: before wait, s = %d\n", me, s);

      shmem_wait_nb (h);

      s = shmem_test_nb (h);

      fprintf (stderr, "%d: after wait, s = %d\n", me, s);
    }

  shmem_barrier_all ();

  fprintf (stderr, "%d: dst = %d\n", me, *dst);

  return 0;
}
