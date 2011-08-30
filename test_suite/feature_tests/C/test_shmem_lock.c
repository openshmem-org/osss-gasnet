/* (c) 2011 University of Houston System.  All rights reserved. */

/* Tests shmem_set_lock, shmem_test_lock
 * and shmem_clear_lock calls*/

#include <stdio.h>
#include <stdlib.h>
#include <mpp/shmem.h>

long L = 0;
int x;

int
main(int argc, char **argv)
{
  int me, npes;
  int slp;
  int ret_val;
  int new_val;

  start_pes(0);
  me = _my_pe();
  npes = _num_pes();
  x=0;
  ret_val = -1;

  if(npes > 1){

    slp = 1;
    shmem_barrier_all();

    shmem_set_lock(&L);

    shmem_int_get(&new_val,&x,1,0);
    new_val++;
    shmem_int_put(&x,&new_val,1,0);  /* increment x on PE 0 */
    shmem_quiet; 

    shmem_clear_lock(&L);

    shmem_barrier_all();
 
    if(me == 0){
      if(x == npes)
        printf("Test for set, and clear lock: Passed\n");
      else
        printf("Test for set, and clear lock: Failed\n");
      x=0;
    }
    shmem_barrier_all();

      do {
        ret_val = shmem_test_lock(&L);
      } while ( ret_val == 1 );

    shmem_int_get(&new_val,&x,1,0);
    new_val++;
    shmem_int_put(&x,&new_val,1,0);  /* increment x on PE 0 */
    shmem_quiet; 

    shmem_clear_lock(&L);

    shmem_barrier_all();

    if(me == 0){
      if(x == npes)
        printf("Test for test lock: Passed\n");
      else
        printf("Test for test lock: Failed\n");

    }
    shmem_barrier_all();
  }
  else
    printf("Number of PEs must be > 1 to test locks, test skipped\n");
  return 0;
}
