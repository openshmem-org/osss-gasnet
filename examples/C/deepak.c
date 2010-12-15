#include <stdio.h>

#include <mpp/shmem.h>

int
main(void)
{
  int *a, *b;

  start_pes(0);

  if (_my_pe() == 0) {
    a = (int *) shmalloc( sizeof(int) );
    printf("addr a = %p\n", a);
  }
  else {
    a = (int *) shmalloc( sizeof(int) );
    printf("addr b = %p\n", b);
    shmem_int_p(b, 999, 0);
  }

  shmem_barrier_all();

  if (_my_pe() == 0) {
    printf("%d: a = %d\n", _my_pe(), *a);
  }
  else {
    printf("%d: b = %d\n", _my_pe(), *b);
  }

  return 0;
}
