/* (c) 2011 University of Houston System.  All rights reserved. */

/* Test whether various types of variables are accessible
 * Test if all PEs are accessible
 */

#include <stdio.h>
#include <mpp/shmem.h>
  
static int
check_it(void *addr)
{
  return shmem_addr_accessible(addr, 1);
}

long global_target;
static int static_target;

int
main(int argc, char *argv[])
{
  long local_target;
  int *shm_target;
  char *msg = "Test Address Accessible: Passed";
  int me,npes,i;
  int pe_acc_success=0;

  start_pes(0);
  me = _my_pe();
  npes = _num_pes();

  shm_target = (int *) shmalloc(sizeof(int));

  shmem_barrier_all();

  if (me == 0) {

    if (! check_it(&global_target)) { /* long global: yes */
      printf("Test Global Address Accessible: Failed\n");
    }
    else{
      printf("Test Global Address Accessible: Passed\n");  
    }
    if (! check_it(&static_target)) { /* static int global: yes */
      printf("Test Static Global Address Accessible: Failed\n");
    }
    else{
      printf("Test Static Global Address Accessible: Passed\n");  
    }
    if (check_it(&local_target)) { /* main() stack: no  */
      printf("Test Stack Address Accessible: Failed\n");

    }
    else{
      printf("Test Stack Address Accessible: Passed\n");  
    }
    if (! check_it(shm_target)) { /* shmalloc: yes */

      printf("Test Shmalloc-ed Address Accessible: Failed\n");
    }
    else{
      printf("Test Shmalloc-ed Address Accessible: Passed\n");  
    }


    for(i=1;i<npes;i++){

      if(shmem_pe_accessible(i)!=1){
        pe_acc_success=1;
      }

    }
    if(pe_acc_success==1){
      printf("Test shmem_pe_accessible: Failed\n");
    }
    else{
      printf("Test shmem_pe_accessible: Passed\n");  
    }


  }

  shfree(shm_target);

  return 0;
}
