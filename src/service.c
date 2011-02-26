#define _POSIX_C_SOURCE 199309
#include <time.h>		/* for nanosleep */
#undef _POSIX_C_SOURCE

#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "comms.h"
#include "trace.h"
#include "state.h"

#include "service.h"

/*
 * TODO: should be user-controllable.  Could also make the library
 * monitor frequency of communication and allow it to fine-tune its
 * own polling speed.
 *
 */
static double backoff_secs = 0.9999;
// static int num_polls_per_loop = 10;

static struct timespec backoff;

static pthread_t service_thr;

/*
 * unused
 *
 */
void
__shmem_service_set_pause(double ms)
{
  double s = floor(ms);
  double f = ms - s;

  backoff.tv_sec  = (long) s;
  backoff.tv_nsec = (long) (f * 1.0e9);
}

/*
 * the real service thread: poll the network.  Should we include a
 * short refractory period to help other things happen in parallel?
 *
 */

static volatile poll_mode_t mode;

void
__shmem_service_set_mode(poll_mode_t m)
{
  mode = m;
}

static
void *
service_thread(void *unused_arg)
{
  while (mode != SERVICE_FINISH) {

    if (mode == SERVICE_POLL) {
      __comms_poll();
    }
    else if (mode == SERVICE_FENCE) {
      __comms_fence();
    }
    else {
      /* TODO: shouldn't get here */
      break;
    }

    // nanosleep(& backoff, (struct timespec *) NULL);

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

  mode = SERVICE_POLL;

  /* set the refractory period */
  __shmem_service_set_pause(backoff_secs);

  s = pthread_create(& service_thr, NULL, service_thread, NULL);
  if (s != 0) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: can't create network service thread (%s)",
		  strerror(errno)
		  );
    /* NOT REACHED */
  }
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

  mode = SERVICE_FINISH;

  s = pthread_join(service_thr, NULL);
  if (s != 0) {
    __shmem_trace(SHMEM_LOG_NOTICE,
		  "internal error: couldn't clean up network service thread (%s)",
		  strerror(errno)
		  );
  }
}
