/*
 *
 * Copyright (c) 2011 - 2013
 *   University of Houston System and Oak Ridge National Laboratory.
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
 * o Neither the name of the University of Houston System, Oak Ridge
 *   National Laboratory nor the names of its contributors may be used to
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
  const shmem_trace_t level;	/* SHMEM_LOG_XXX symbol for logging */
  const char *text;		/* human readable name */
  shmem_trace_state_t state;	/* off or on */
} trace_table_t;

#ifdef HAVE_FEATURE_TRACE

#define INIT_STATE(L, State) { SHMEM_LOG_##L , #L , State }

static
trace_table_t
tracers[] =
  {
    INIT_STATE (FATAL, ON),

    INIT_STATE (SYMBOLS, OFF),
    INIT_STATE (DEBUG, OFF),
    INIT_STATE (INFO, OFF),
    INIT_STATE (VERSION, OFF),

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
    INIT_STATE (MODULES, OFF),
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

static
int
__shmem_trace_enable_text (char *trace)
{
  int i;
  trace_table_t *t = tracers;

  for (i = 0; i < n_tracers; i += 1)
    {
      if (strcasecmp (trace, t->text) == 0)
	{
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

static
void
__shmem_trace_enable_all (void)
{
  int i;
  trace_table_t *t = tracers;

  for (i = 0; i < n_tracers; i += 1)
    {
      t->state = ON;
      t += 1;
    }
}

/**
 * translate the message level to text description
 *
 */

static
const char *
__level_to_string (shmem_trace_t level)
{
  int i;
  trace_table_t *t = tracers;

  for (i = 0; i < n_tracers; i += 1)
    {
      if (level == t->level)
	{
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
__shmem_trace_is_enabled (shmem_trace_t level)
{
  int i;
  trace_table_t *t = tracers;

  for (i = 0; i < n_tracers; i += 1)
    {
      if (level == t->level)
	{
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

static void
logging_filestream_init (void)
{
  char *shlf;
  FILE *fp;

  trace_log_stream = stderr;

  shlf = __shmem_comms_getenv (SHMEM_LOGFILE_ENVVAR);
  if (shlf == (char *) NULL)
    {
      return;
    }

  fp = fopen (shlf, "a");
  if (fp != (FILE *) NULL)
    {
      trace_log_stream = fp;
    }
}

/**
 * parse any environment settings and set up logging output
 *
 */

static void
sgi_compat_environment_init (void)
{
  int overwrite = 1;
  char *sma;

  /* this one "prints out copious data" so turn on all debugging */
  sma = __shmem_comms_getenv ("SMA_DEBUG");
  if (sma != (char *) NULL)
    {
      (void) setenv ("SHMEM_LOG_LEVELS", "all", overwrite);
    }
  /* this one shows information about env vars, pass through */
  sma = __shmem_comms_getenv ("SMA_INFO");
  if (sma != (char *) NULL)
    {
      (void) setenv ("SHMEM_LOG_LEVELS", "info", overwrite);
    }
  /* turn on symmetric memory debugging */
  sma = __shmem_comms_getenv ("SMA_MALLOC_DEBUG");
  if (sma != (char *) NULL)
    {
      (void) setenv ("SHMEM_LOG_LEVELS", "memory", overwrite);
    }
  /* version information.  "version" trace facility can cover this */
  sma = __shmem_comms_getenv ("SMA_VERSION");
  if (sma != (char *) NULL)
    {
      (void) setenv ("SHMEM_LOG_LEVELS", "version", overwrite);
    }
  /* if heap size given, translate into our env var */
  sma = __shmem_comms_getenv ("SMA_SYMMETRIC_SIZE");
  if (sma != (char *) NULL)
    {
      (void) setenv ("SHMEM_SYMMETRIC_HEAP_SIZE", sma, overwrite);
    }
}

/**
 * check environment for settings, and enable listed levels
 */

static
void
parse_log_levels (void)
{
  char *shll = __shmem_comms_getenv (SHMEM_LOGLEVELS_ENVVAR);
  const char *delims = ",:;";
  char *opt = strtok (shll, delims);

  if (shll != (char *) NULL)
    {
      while (opt != (char *) NULL)
	{
	  if (strcasecmp (opt, "all") == 0)
	    {
	      __shmem_trace_enable_all ();
	      break;
	      /* NOT REACHED */
	    }
	  (void) __shmem_trace_enable_text (opt);
	  opt = strtok ((char *) NULL, delims);
	}
    }
}

/**
 * spit out information about environment variables known, if
 * requested
 */

#define INFO_MSG(Var, Text)			\
  __shmem_trace (SHMEM_LOG_INFO,		\
		 "%-28s %s",			\
		 Var,				\
		 Text				\
		 );

void
__shmem_maybe_tracers_show_info (void)
{
  /* only "master" dumps this out */
  if (GET_STATE (mype) != 0)
    {
      return;
    }

  if (! __shmem_trace_is_enabled (SHMEM_LOG_INFO))
    {
      return;
    }

  __shmem_trace (SHMEM_LOG_INFO,
		 "We understand these SGI compatibility environment variables:");
  INFO_MSG ("SMA_VERSION", "Print OpenSHMEM version.");
  INFO_MSG ("SMA_INFO", "Print this message.");
  INFO_MSG ("SMA_SYMMETRIC_SIZE",
	    "Specify the size in bytes of symmetric memory.");
  INFO_MSG ("SMA_DEBUG", "Print internal debug information.");
  __shmem_trace (SHMEM_LOG_INFO, "");
  __shmem_trace (SHMEM_LOG_INFO,
		 "We also understand these new environment variables:");
  INFO_MSG ("SHMEM_LOG_LEVELS",
	    "Select which kinds of trace messages are enabled");
  INFO_MSG ("SHMEM_LOG_FILE",
	    "Filename to append trace output to, instead of standard error");
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
__shmem_tracers_init (void)
{
  logging_filestream_init ();

  sgi_compat_environment_init ();

  parse_log_levels ();
}

/**
 * spit out which message levels are active
 *
 * OK to use public API here since we're only called when initialized
 *
 */

void
__shmem_tracers_show (void)
{
  if (__shmem_trace_is_enabled (SHMEM_LOG_INIT))
    {
      char buf[TRACE_MSG_BUF_SIZE];
      char *p = buf;
      int i;
      trace_table_t *t = tracers;

      strcpy (p, "Enabled Messages: ");

      for (i = 0; i < n_tracers; i += 1)
	{
	  if (t->state == ON)
	    {
	      strncat (p, t->text, strlen (t->text));
	      strncat (p, " ", 1);
	    }
	  t += 1;
	}
      __shmem_trace (SHMEM_LOG_INIT, p);
    }
}

/**
 * produce a trace message
 */

void
__shmem_trace (shmem_trace_t msg_type, char *fmt, ...)
{
  if (__shmem_trace_is_enabled (msg_type))
    {
      char tmp1[TRACE_MSG_BUF_SIZE];
      char tmp2[TRACE_MSG_BUF_SIZE];
      va_list ap;

      snprintf (tmp1, TRACE_MSG_BUF_SIZE,
		"%-8.8f PE %d: %s: ",
		__shmem_elapsed_clock_get (),
		GET_STATE (mype), __level_to_string (msg_type));

      va_start (ap, fmt);
      vsnprintf (tmp2, TRACE_MSG_BUF_SIZE, fmt, ap);
      va_end (ap);

      strncat (tmp1, tmp2, strlen (tmp2));
      strncat (tmp1, "\n", 1);

      fputs (tmp1, trace_log_stream);
      fflush (trace_log_stream);	/* make sure this all goes out in 1 burst */

      if (msg_type == SHMEM_LOG_FATAL)
	{
	  __shmem_exit (1);
	  /* NOT REACHED */
	}
    }
}

#else /* HAVE_FEATURE_TRACE */

void
__shmem_tracers_init (void)
{
}
void
__shmem_tracers_show (void)
{
}
void
__shmem_maybe_tracers_show_info (void)
{
}
void
__shmem_trace (shmem_trace_t msg_type, char *fmt, ...)
{
}
int
__shmem_trace_is_enabled (shmem_trace_t level)
{
  return 0;
}

#endif /* HAVE_FEATURE_TRACE */
