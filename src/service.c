#include <stdio.h>
#include <pthread.h>

#include "comms.h"

static pthread_t service_thr;

volatile int __shmem_is_polling;

static
void *
service_thread(void *unused_arg)
{
  while (__shmem_is_polling) {
    __comms_poll();
  }
  return (void *) NULL;
}

void
__shmem_service_thread_init(void)
{
  __shmem_is_polling = 1;
  pthread_create(& service_thr, NULL, service_thread, NULL);
}

void
__shmem_service_thread_finalize(void)
{
  __shmem_is_polling = 0;
  pthread_join(service_thr, NULL);
}
