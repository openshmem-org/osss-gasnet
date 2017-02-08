/*
 *
 * Copyright (c) 2016
 *   Stony Brook University
 * Copyright (c) 2015 - 2016
 *   Los Alamos National Security, LLC.
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
 *   this list of conditions and the following disclaimers.
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
 * o Neither the name of Los Alamos National Security, LLC, Los Alamos
 *   National Laboratory, LANL, the U.S. Government, nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
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


#define _GNU_SOURCE 1

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>

#include "state.h"
#include "trace.h"
#include "clock.h"
#include "utils.h"
#include "updown.h"

#include "comms/comms.h"

#define SHMEM_LOGLEVELS_ENVVAR "SHMEM_LOG_LEVELS"
#define SHMEM_LOGFILE_ENVVAR   "SHMEM_LOG_FILE"

/**
 * tracing states
 */

typedef enum
{
    OFF = 0,
    ON,
} shmem_trace_state_t;

/**
 * trace levels and states
 */

typedef struct
{
    const shmem_trace_t level;  /* SHMEM_LOG_XXX symbol for logging */
    const char *text;           /* human readable name */
    shmem_trace_state_t state;  /* off or on */
} trace_table_t;

#ifdef HAVE_FEATURE_TRACE

#define INIT_STATE(L, State) { SHMEM_LOG_##L , #L , State }

static trace_table_t tracers[] = {
    INIT_STATE (FATAL, ON),
    INIT_STATE (SYMBOLS, OFF),
    INIT_STATE (DEBUG, OFF),
    INIT_STATE (INFO, OFF),
    INIT_STATE (INIT, OFF),
    INIT_STATE (FINALIZE, OFF),
    INIT_STATE (ATOMIC, OFF),
    INIT_STATE (AUTH, OFF),
    INIT_STATE (BARRIER, OFF),
    INIT_STATE (BROADCAST, OFF),
    INIT_STATE (REDUCTION, OFF),
    INIT_STATE (CACHE, OFF),
    INIT_STATE (COLLECT, OFF),
    INIT_STATE (FENCE, OFF),
    INIT_STATE (QUIET, OFF),
    INIT_STATE (LOCK, OFF),
    INIT_STATE (MEMORY, OFF),
    INIT_STATE (NOTICE, OFF),
    INIT_STATE (SERVICE, OFF),
    INIT_STATE (PROFILING, OFF),
};

static const int n_tracers = TABLE_SIZE (tracers);

/**
 * big enough?  I reckon so, we're not writing a novel...
 *
 * TODO: alternatively, can loop on increasing buffer size on
 * overflow...better way to do it?  Or is it worth it?
 *
 */

#define TRACE_MSG_BUF_SIZE 256

/**
 * enable the named message category.  Return 1 if matched,
 * 0 if not found
 *
 */

static inline int
shmemi_trace_enable_text (char *trace)
{
    int i;
    trace_table_t *t = tracers;

    for (i = 0; i < n_tracers; i += 1) {
        if (strcasecmp (trace, t->text) == 0) {
            t->state = ON;
            return 1;
            /* NOT REACHED */
        }
        t += 1;
    }
    return 0;
}

/**
 * enable all message categories
 */

static inline void
shmemi_trace_enable_all (void)
{
    int i;
    trace_table_t *t = tracers;

    for (i = 0; i < n_tracers; i += 1) {
        t->state = ON;
        t += 1;
    }
}

/**
 * translate the message level to text description
 *
 */

static inline const char *
level_to_string (shmem_trace_t level)
{
    int i;
    trace_table_t *t = tracers;

    for (i = 0; i < n_tracers; i += 1) {
        if (level == t->level) {
            return t->text;
            /* NOT REACHED */
        }
        t += 1;
    }
    return "?";
}

/* -- end of private routines -- */

/**
 * is the trace STATE currently enabled?
 */

int
shmemi_trace_is_enabled (shmem_trace_t level)
{
    int i;
    trace_table_t *t = tracers;

    for (i = 0; i < n_tracers; i += 1) {
        if (level == t->level) {
            return (t->state == ON);
            /* NOT REACHED */
        }
        t += 1;
    }
    return 0;
}

/**
 * where to send trace output
 */

static FILE *trace_log_stream;

/**
 * default log to stderr.  if env var set, try to append to that file
 * instead, fall back to stderr if not
 *
 */

static inline void
logging_filestream_init (void)
{
    char *shlf;
    FILE *fp;

    trace_log_stream = stderr;

    shlf = shmemi_comms_getenv (SHMEM_LOGFILE_ENVVAR);
    if (shlf == NULL) {
        return;
    }

    fp = fopen (shlf, "a");
    if (fp != NULL) {
        trace_log_stream = fp;
    }
}

/**
 * turn off tracers at end
 *
 */

static inline void
logging_filestream_fini (void)
{
    fclose (trace_log_stream);
}

/**
 * parse any environment settings and set up logging output
 *
 * 1.3 proposes renaming SMA_xxx to SHMEM_xxx.  We had a few of our
 * own SHMEM_ before anyone tried to standardize.  This should handle
 * old/non-standard ones.
 *
 */

static inline void
compat_environment_init (void)
{
    int overwrite = 1;
    char *shs;

    /* this one "prints out copious data" so turn on all debugging */
    if ( (shmemi_comms_getenv ("SHMEM_DEBUG") != NULL) ||
         (shmemi_comms_getenv ("SMA_DEBUG") != NULL) ) {
        (void) setenv ("SHMEM_LOG_LEVELS", "all", overwrite);
    }

    /* this one shows information about env vars, pass through */
    if ( (shmemi_comms_getenv ("SHMEM_INFO") != NULL) ||
         (shmemi_comms_getenv ("SMA_INFO") != NULL) ) {
        (void) setenv ("SHMEM_LOG_LEVELS", "info", overwrite);
    }

    /* version information */
    if ( (shmemi_comms_getenv ("SHMEM_VERSION") != NULL) ||
         (shmemi_comms_getenv ("SMA_VERSION") != NULL) ) {
        (void) setenv ("SHMEM_LOG_LEVELS", "init", overwrite);
    }

    /* if heap size given, translate into our env var */
    shs = shmemi_comms_getenv ("SHMEM_SYMMETRIC_SIZE");
    if (shs == NULL) {
        shs = shmemi_comms_getenv ("SMA_SYMMETRIC_SIZE");
        if (shs != NULL) {
            (void) setenv ("SHMEM_SYMMETRIC_HEAP_SIZE", shs, overwrite);
        }
    }
}

/**
 * check environment for settings, and enable listed levels
 */

static inline void
parse_log_levels (void)
{
    char *shll = shmemi_comms_getenv (SHMEM_LOGLEVELS_ENVVAR);
    const char *delims = ",:;";
    char *opt = strtok (shll, delims);

    if (shll != NULL) {
        while (opt != NULL) {
            if (strcasecmp (opt, "all") == 0) {
                shmemi_trace_enable_all ();
                break;
                /* NOT REACHED */
            }
            (void) shmemi_trace_enable_text (opt);
            opt = strtok (NULL, delims);
        }
    }
}

/**
 * spit out information about environment variables known, if
 * requested
 */

#define INFO_MSG(Var, Text)                     \
    shmemi_trace (SHMEM_LOG_INFO,               \
                  "%-28s %s",                   \
                  Var,                          \
                  Text                          \
                  );

void
shmemi_maybe_tracers_show_info (void)
{
    /* only "master" dumps this out */
    if (GET_STATE (mype) != 0) {
        return;
    }

    if (!shmemi_trace_is_enabled (SHMEM_LOG_INFO)) {
        return;
    }

    shmemi_trace (SHMEM_LOG_INFO,
                  "We understand these SGI compatibility"
                  " environment variables:"
                  );
    INFO_MSG ("{SHMEM,SMA}_VERSION", "Print OpenSHMEM version.");
    INFO_MSG ("{SHMEM,SMA}_INFO", "Print this message.");
    INFO_MSG ("{SHMEM,SMA}_SYMMETRIC_SIZE",
              "Specify the size in bytes of symmetric memory.");
    INFO_MSG ("{SHMEM,SMA}_DEBUG", "Print internal debug information.");

    shmemi_trace (SHMEM_LOG_INFO, "");
    shmemi_trace (SHMEM_LOG_INFO,
                  "We also understand these new environment variables:");

    INFO_MSG ("SHMEM_LOG_LEVELS",
              "Select which kinds of trace messages are enabled");
    INFO_MSG ("SHMEM_LOG_FILE",
              "Filename to append trace output to,"
              " instead of standard error");
    INFO_MSG ("SHMEM_SYMMETRIC_HEAP_SIZE",
              "Specify the size of symmetric memory.");
    INFO_MSG ("SHMEM_BARRIER_ALL_ALGORITHM",
              "Choose shmem_barrier_all implementation.");
    INFO_MSG ("SHMEM_BARRIER_ALGORITHM",
              "Choose shmem_barrier implementation.");
    INFO_MSG ("SHMEM_PE_ACCESSIBLE_TIMEOUT",
              "How long to wait for PE accessibility check.");
}

/**
 * enable the tracers sub-system
 */

void
shmemi_tracers_init (void)
{
    logging_filestream_init ();

    compat_environment_init ();

    parse_log_levels ();
}

/**
 * shut down the tracers sub-system
 */

void
shmemi_tracers_fini (void)
{
    logging_filestream_fini ();
}

/**
 * spit out which message levels are active
 *
 * OK to use public API here since we're only called when initialized
 *
 */

void
shmemi_tracers_show (void)
{
    if (shmemi_trace_is_enabled (SHMEM_LOG_INIT)) {
        char buf[TRACE_MSG_BUF_SIZE];
        char *p = buf;
        unsigned int i;
        trace_table_t *t = tracers;
        const char *enamsg = "Enabled Messages: ";
        /* how many chars are free in the bufer? */
        unsigned int left = TRACE_MSG_BUF_SIZE - 1 - strlen (enamsg);

        strncpy (p, enamsg, left);

        for (i = 0; i < n_tracers; i += 1) {
            if (t->state == ON) {
                strncat (p, t->text, left);
                strncat (p, " ", 1);
                left -= strlen (t->text) + 1;
            }
            t += 1;
        }
        shmemi_trace (SHMEM_LOG_INIT, p);
    }
}

/**
 * produce a trace message
 */

void
shmemi_trace (shmem_trace_t msg_type, char *fmt, ...)
{
    if (shmemi_trace_is_enabled (msg_type)) {
        char tmp1[TRACE_MSG_BUF_SIZE];
        char tmp2[TRACE_MSG_BUF_SIZE];
        va_list ap;

        snprintf (tmp1, TRACE_MSG_BUF_SIZE,
                  "[%-8.8f] PE %d: %s: ",
                  shmemi_elapsed_clock_get (),
                  GET_STATE (mype), level_to_string (msg_type));

        va_start (ap, fmt);
        vsnprintf (tmp2, TRACE_MSG_BUF_SIZE, fmt, ap);
        va_end (ap);

        strncat (tmp1, tmp2, strlen (tmp2));
        strncat (tmp1, "\n", 1);

        fputs (tmp1, trace_log_stream);
        /* make sure this all goes out in 1 burst */
        fflush (trace_log_stream);

        if (msg_type == SHMEM_LOG_FATAL) {
            exit (1);
            /* NOT REACHED */
        }
    }
}

#else /* HAVE_FEATURE_TRACE */

void
shmemi_tracers_init (void)
{
}

void
shmemi_tracers_fini (void)
{
}

void
shmemi_tracers_show (void)
{
}

void
shmemi_maybe_tracers_show_info (void)
{
}

void
shmemi_trace (shmem_trace_t msg_type, char *fmt, ...)
{
}

int
shmemi_trace_is_enabled (shmem_trace_t level)
{
    return 0;
}

#endif /* HAVE_FEATURE_TRACE */
