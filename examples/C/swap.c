#include <stdio.h>

#include <shmem.h>

int
main()
{
  long *target;
  int me, npes;
  long swapped_val, new_val;

  shmem_init();
  me = shmem_my_pe();
  npes = shmem_num_pes();

  target = (long *) shmalloc( sizeof(*target) );

  *target = me;
  shmem_barrier_all();

  new_val = me;

  if (me & 1) {
    swapped_val = shmem_long_swap(target, new_val, (me + 1) % npes);
    printf("%d: target = %d, swapped = %d\n", me, *target, swapped_val);
  }


  return 0;
}
