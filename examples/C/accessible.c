#include <stdio.h>
#include <mpp/shmem.h>

static void
check_it(void *addr)
{
  int me = _my_pe();
  int a = shmem_addr_accessible(addr, me);

  printf("%d: variable @ %p is %sglobal\n",
         me, addr,
         a ? "" : "not "
        );
}

long global_target;
static int static_target;

int
main(int argc, char *argv[])
{
  long local_target;

  start_pes(0);

  /* should work */
  check_it(&global_target);

  /* should work */
  check_it(&static_target);

  /* should fail */
  check_it(&local_target);

  return 0;
}
