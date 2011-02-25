#define _POSIX_C_SOURCE 199309
#include <time.h>

#include <stdio.h>
#include <pthread.h>

#include "comms.h"
#include "trace.h"
#include "state.h"

static pthread_t service_thr;

static volatile int polling;

/*
 * TODO: should be user-controllable
 */

static long backoff_millisecs = 1000L;

static struct timespec backoff;

/*
 * the real service thread: poll the network.  We include a short
 * refractory period to help other things happen in parallel.
 *
 */

static
void *
service_thread(void *unused_arg)
{
  while (polling) {
    __comms_poll();
    nanosleep(& backoff, (struct timespec *) NULL);
  }
  return (void *) NULL;
}

/*
 * start the service sub-system.  Initiate polling sentinel and get
 * the service thread going.  Fatal error if we cant create the
 * thread.
 *
 */

void
__shmem_service_thread_init(void)
{
  int s;

  polling = 1;

  /* set the refractory period */
  backoff.tv_sec = 0;
  backoff.tv_nsec = backoff_millisecs * 1000000L;

  s = pthread_create(& service_thr, NULL, service_thread, NULL);
  if (s != 0) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: can't create network service thread"
		  );
    /* NOT REACHED */
  }
}

/*
 * stop the service sub-system.  Disable polling sentinel and reap the
 * service thread.  If we can't shut down the thread, we'll complain
 * but we'll continue with the rest of the shut down.
 *
 */

void
__shmem_service_thread_finalize(void)
{
  int s;

  polling = 0;

  s = pthread_join(service_thr, NULL);
  if (s != 0) {
    __shmem_trace(SHMEM_LOG_NOTICE,
		  "internal error: couldn't clean up network service thread"
		  );
  }
}
