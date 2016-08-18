/*
 *
 * Copyright (c) 2011 - 2016
 *   University of Houston System and UT-Battelle, LLC.
 * Copyright (c) 2009 - 2016
 *   Silicon Graphics International Corp.  SHMEM is copyrighted
 *   by Silicon Graphics International Corp. (SGI) The OpenSHMEM API
 *   (shmem) is released by Open Source Software Solutions, Inc., under an
 *   agreement with Silicon Graphics International Corp. (SGI).
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * o Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * o Neither the name of the University of Houston System,
 *   UT-Battelle, LLC. nor the names of its contributors may be used to
 *   endorse or promote products derived from this software without specific
 *   prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#include "state.h"
#include "trace.h"

#include "comms/comms.h"

#if 0
static const char *ping_timeout_envvar = "SHMEM_PE_ACCESSIBLE_TIMEOUT";
static const double DEFAULT_PE_ACCESSIBLE_TIMEOUT = 1.0;

static struct itimerval zero;
#endif

#if 0
/**
 * Set the timer (we're not using the interval)
 *
 */
static void
assign_timer (long sec, long usec, struct itimerval *ivp)
{
    ivp->it_value.tv_sec = sec;
    ivp->it_value.tv_usec = usec;

    ivp->it_interval.tv_sec = 0;
    ivp->it_interval.tv_usec = 0;
}
#endif

#if 0
/**
 * split human-readable time into timer struct
 *
 */
static void
parse_alarm_time (double ts, struct itimerval *ivp)
{
    double s, f;

    s = floor (ts);             /* seconds */
    f = ts - s;                 /* fractional part */

    assign_timer ((long) s, 1000000L * f, ivp);
}
#endif

/**
 * set the time out.  Can also be used to tune library behavior
 *
 */
void
shmemi_set_ping_timeout (double secs)
{
#if 0
    parse_alarm_time (secs, &GET_STATE (ping_timeout));

    shmemi_trace (SHMEM_LOG_INIT,
                  "PE accessibility timeout set to %f sec", secs);
#endif
}

/**
 * initialize the ping sub-system.  Respect any environment setting
 * asking for different ping timeout.
 *
 */
void
shmemi_ping_init (void)
{
#if 0
    double timeout = DEFAULT_PE_ACCESSIBLE_TIMEOUT;
    char *pt = shmemi_comms_getenv (ping_timeout_envvar);

    if (pt != (char *) NULL) {
        timeout = atof (pt);
    }

    /* sanity check it */
    if (timeout < 0.0) {
        double ot = timeout;
        timeout = DEFAULT_PE_ACCESSIBLE_TIMEOUT;
        shmemi_trace (SHMEM_LOG_INIT,
                      "PE accessibility timeout %f negative,"
                      " reset to default %f sec",
                      ot, timeout);
    }

    shmemi_set_ping_timeout (timeout);

    assign_timer (0, 0, &zero);
#endif
}

/**
 * set up the alarm before doing a ping
 *
 */
void
shmemi_ping_set_alarm (void)
{
#if 0
    int s = setitimer (ITIMER_REAL, &GET_STATE (ping_timeout), NULL);
    if (s != 0) {
        shmemi_trace (SHMEM_LOG_FATAL,
                      "internal error: couldn't set accessibility timer (%s)",
                      strerror (errno));
        return;
        /* NOT REACHED */
    }
#endif
}

/**
 * clear the alarm after doing a ping
 *
 */
void
shmemi_ping_clear_alarm (void)
{
#if 0
    int s = setitimer (ITIMER_REAL, &zero, NULL);
    if (s != 0) {
        shmemi_trace (SHMEM_LOG_FATAL,
                      "internal error: couldn't clear accessibility timer (%s)",
                      strerror (errno));
        return;
        /* NOT REACHED */
    }
#endif
}
