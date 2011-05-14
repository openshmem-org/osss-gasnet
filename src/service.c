#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>
#include <sched.h>

#include "comms.h"
#include "trace.h"
#include "state.h"

#include "service.h"

static pthread_t service_thr;

/*
 * the real service thread: poll the network.  Should we include a
 * short refractory period to help other things happen in parallel?
 *
 */

static volatile poll_mode_t poll_mode;

void
__shmem_service_set_mode(poll_mode_t m)
{
  poll_mode = m;
}

volatile int fence_done = 0;

static
void *
service_thread(void *unused_arg)
{
  int polling = 1;

  while (polling) {

    __shmem_trace(SHMEM_LOG_SERVICE,
		  "poll_mode = %d",
		  poll_mode
		  );

    switch (poll_mode) {

    case SERVICE_POLL:
      __shmem_comms_poll_service();
      break;

    case SERVICE_FENCE:
      __shmem_comms_fence_service();
      __shmem_service_set_mode(SERVICE_POLL);
      fence_done = 1;
      break;

    case SERVICE_FINISH:
      __shmem_comms_fence_service();
      polling = 0;
      break;

    default:
      /* TODO: shouldn't get here */
      break;

    }

  }

  return (void *) NULL;
}

static void
set_low_priority(pthread_attr_t *p)
{
  struct sched_param sp;

  pthread_attr_init(p);
  pthread_attr_getschedparam(p, & sp);
  sp.sched_priority = sched_get_priority_min(SCHED_OTHER);
  pthread_attr_setschedpolicy(p, SCHED_OTHER);
  pthread_attr_setschedparam(p, & sp);
}

/*
 * start the service sub-system.  Initiate polling sentinel and get
 * the service thread going.  Fatal error if we cant create the
 * thread.
 *
 */

static const int max_create_tries = 100;

void
__shmem_service_thread_init(void)
{
  int try;
  pthread_attr_t pa;

  __shmem_service_set_mode(SERVICE_POLL);

  /* prefer the processing thread over the service thread */
  set_low_priority(& pa);

  try = 1;
  while (try < max_create_tries) {
    int s = pthread_create(& service_thr, & pa, service_thread, NULL);
    if (s == 0) {
      break;			/* created thread ok */
    }
    if (errno != EAGAIN) {
      __shmem_trace(SHMEM_LOG_FATAL,
		    "internal error: can't create network service thread (errno=%d, %s)",
		    errno, strerror(errno)
		    );
      /* NOT REACHED */
    }
    try += 1;
  }

  __shmem_trace(SHMEM_LOG_SERVICE,
		"thread started, after %d tr%s",
		try,
		try == 1 ? "y" : "ies"
		);
}

/*
 * stop the service sub-system.  Disable polling sentinel and reap the
 * service thread.  If we can't shut down the thread, complain but
 * continue with the rest of the shut down.
 *
 */

void
__shmem_service_thread_finalize(void)
{
  int s;

  __shmem_comms_barrier_all();

  __shmem_service_set_mode(SERVICE_FINISH);

  s = pthread_join(service_thr, NULL);
  if (s != 0) {
    __shmem_trace(SHMEM_LOG_NOTICE,
		  "internal error: couldn't clean up network service thread (%s)",
		  strerror(errno)
		  );
  }

  __shmem_trace(SHMEM_LOG_SERVICE,
		"thread finished"
		);
}
