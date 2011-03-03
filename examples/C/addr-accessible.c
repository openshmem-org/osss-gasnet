/*
 * test whether various types of variables are accessible
 *
 * oshrun -np 2 ./accessible.x
 *
 *
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
  char *msg = "OK";
  int me;

  start_pes(0);
  me = _my_pe();

  shm_target = (int *) shmalloc(sizeof(int));

  if (me == 0) {

    if (! check_it(&global_target)) { /* long global: yes */
      msg = "FAIL (global long)";
    }
    if (! check_it(&static_target)) { /* static int global: yes */
      msg = "FAIL (static int)";
    }
    if (check_it(&local_target)) { /* main() stack: no  */
      msg = "FAIL (stack variable)";
    }
    if (! check_it(shm_target)) { /* shmalloc: yes */
      msg = "FAIL (shmalloc)";
    }

  printf("%s\n", msg);

  }

  shfree(shm_target);

  return 0;
}
