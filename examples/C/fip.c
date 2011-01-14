/*
 * float value put to shmalloc'ed variable
 *
 */

#include <stdio.h>
#include <math.h>

#include <mpp/shmem.h>

static const float e       = 2.71828182;

static const float epsilon = 0.00000001;

int
main(void)
{
  float *f;
  int me;

  start_pes(0);
  me = _my_pe();

  f = (float *) shmalloc( sizeof(*f) );

  *f = 3.1415927;
  shmem_barrier_all();

  if (me == 0) {
    shmem_float_p(f, e, 1);
  }

  shmem_barrier_all();

  if (me == 1) {
    if (fabs(*f - e) > epsilon) {
      printf("FAIL\n");
    }
  }

  return 0;
}
