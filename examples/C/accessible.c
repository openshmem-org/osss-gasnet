/*
 * test whether global variables are accessible
 *
 * oshrun -np 1 ./accessible.x
 *
 *
 */

#include <stdio.h>
#include <mpp/shmem.h>

static void
check_it(char *descr, void *addr)
{
  int me = _my_pe();
  int a = shmem_addr_accessible(addr, (me + 1) % _num_pes());

  printf("%d: %-20s variable @ %016p is %sglobal\n",
         me, descr, addr,
         a ? "" : "NOT "
        );
}

long global_target;
static int static_target;

int
main(int argc, char *argv[])
{
  long local_target;
  int *shm_target;

  start_pes(0);

  shm_target = (int *) shmalloc(sizeof(int));

  check_it("long global",       &global_target);      /* yes */
  check_it("static int global", &static_target);      /* yes */
  check_it("local to main()",   &local_target);       /* no  */
  check_it("shmalloc'ed int",   shm_target);          /* yes */

  shfree(shm_target);

  return 0;
}
