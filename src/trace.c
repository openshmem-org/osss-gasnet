#define _GNU_SOURCE 1

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>

#include "state.h"
#include "trace.h"
#include "updown.h"
#include "comms.h"
#include "clock.h"

static const char *shmem_loglevels_envvar = "SHMEM_LOG_LEVELS";

static const char *shmem_logfile_envvar = "SHMEM_LOG_FILE";

typedef enum {
  OFF=0,
  ON,
} shmem_trace_state_t;		/* tracing states */

typedef struct {
  const shmem_trace_t level;	/* SHMEM_LOG_XXX symbol for logging */
  const char *text;		/* human readable name */
  shmem_trace_state_t state;	/* off or on */
} trace_table_t;

#define INIT_LEVEL(L, State) { SHMEM_LOG_##L , #L , State }

static
trace_table_t tracers[] =
  {
    INIT_LEVEL(FATAL,      ON ),

    INIT_LEVEL(VERSION,    OFF),
    INIT_LEVEL(DEBUG,  	   OFF),
    INIT_LEVEL(INFO,   	   OFF),
    INIT_LEVEL(AUTH,   	   OFF),
    INIT_LEVEL(NOTICE, 	   OFF),
    INIT_LEVEL(INIT,   	   OFF),
    INIT_LEVEL(MEMORY, 	   OFF),
    INIT_LEVEL(CACHE,  	   OFF),
    INIT_LEVEL(BARRIER,    OFF),
    INIT_LEVEL(BROADCAST,  OFF),
    INIT_LEVEL(COLLECT,    OFF),
    INIT_LEVEL(REDUCE,     OFF),
    INIT_LEVEL(SYMBOLS,    OFF),
    INIT_LEVEL(LOCK,       OFF),
    INIT_LEVEL(SERVICE,    OFF),
    INIT_LEVEL(QUIET,      OFF),
    INIT_LEVEL(FENCE,      OFF),
  };
static const int n_tracers = sizeof(tracers) / sizeof(tracers[0]);


/*
 * big enough?  I reckon so, we're not writing a novel...
 *
 * TODO: alternatively, can loop on increasing buffer size on
 * overflow...better way to do it?  Or is it worth it?
 *
 */

#define TRACE_MSG_BUF_SIZE 256

/*
 * enable the named message category.  Return 1 if matched,
 * 0 if not found
 *
 */

static int
__shmem_trace_enable_text(char *trace)
{
  int i;
  trace_table_t *t = tracers;

  for (i = 0; i < n_tracers; i += 1) {
    if (strcasecmp(trace, t->text) == 0) {
      t->state = ON;
      return 1;
      /* NOT REACHED */
    }
    t += 1;
  }
  return 0;
}

static void
__shmem_trace_enable_all(void)
{
  int i;
  trace_table_t *t = tracers;

  for (i = 0; i < n_tracers; i += 1) {
    t->state = ON;
    t += 1;
  }
}

/*
 * translate the message level to text description
 *
 */

static const char *
__level_to_string(shmem_trace_t level)
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

/* -- end of static -- */

int
__shmem_trace_is_enabled(shmem_trace_t level)
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

static FILE *trace_log_stream;

/*
 * default log to stderr.  if env var set, try to append to that file
 * instead, fall back to stderr if not
 *
 */

static void
logging_filestream_init(void)
{
  char *shlf;
  FILE *fp;

  trace_log_stream = stderr;

  shlf = __shmem_comms_getenv(shmem_logfile_envvar);
  if (shlf == (char *) NULL) {
    return;
  }

  fp = fopen(shlf, "a");
  if (fp != (FILE *) NULL) {
    trace_log_stream = fp;
  }
}

/*
 * parse any environment settings and set up logging output
 *
 */

static void
sgi_compat_environment_init(void)
{
  int overwrite = 1;
  char *sma;

  /* this one "prints out copious data" so turn on all debugging */
  sma = __shmem_comms_getenv("SMA_DEBUG");
  if (sma != (char *) NULL) {
    (void) setenv("SHMEM_LOG_LEVELS",
		  "all",
		  overwrite
		  );
  }
  /* this one shows information about env vars, pass through */
  sma = __shmem_comms_getenv("SMA_INFO");
  if (sma != (char *) NULL) {
    (void) setenv("SHMEM_LOG_LEVELS",
		  "info",
		  overwrite
		  );
  }
  /* turn on symmetric memory debugging */
  sma = __shmem_comms_getenv("SMA_MALLOC_DEBUG");
  if (sma != (char *) NULL) {
    (void) setenv("SHMEM_LOG_LEVELS",
		  "memory",
		  overwrite
		  );
  }
  /* version information.  "version" trace facility can cover this */
  sma = __shmem_comms_getenv("SMA_VERSION");
  if (sma != (char *) NULL) {
    (void) setenv("SHMEM_LOG_LEVELS",
		  "version",
		  overwrite
		  );
  }
  /* if heap size given, translate into our env var */
  sma = __shmem_comms_getenv("SMA_SYMMETRIC_SIZE");
  if (sma != (char *) NULL) {
    (void) setenv("SHMEM_SYMMETRIC_HEAP_SIZE",
		  sma,
		  overwrite
		  );
  }
}

static void
parse_log_levels(void)
{
  char *shll = __shmem_comms_getenv(shmem_loglevels_envvar);
  const char *delims = ",:;";
  char *opt = strtok(shll, delims);

  if (shll != (char *) NULL) {
    while (opt != (char *) NULL) {
      if (strcasecmp(opt, "all") == 0) {
	__shmem_trace_enable_all();
	break;
	/* NOT REACHED */
      }
      (void) __shmem_trace_enable_text(opt);
      opt = strtok((char *) NULL, delims);
    }
  }
}

#define INFO_MSG(Var, Text)			\
  __shmem_trace(SHMEM_LOG_INFO,			\
		"%-28s %s",			\
		Var,				\
		Text				\
		);

void
__shmem_maybe_tracers_show_info(void)
{
  /* only "master" dumps this out */
  if (GET_STATE(mype) != 0) {
    return;
  }

  if (! __shmem_trace_is_enabled(SHMEM_LOG_INFO)) {
    return;
  }

  __shmem_trace(SHMEM_LOG_INFO,
		"We understand these SGI compatibility environment variables:"
		);
  INFO_MSG("SMA_VERSION",
	   "Print OpenSHMEM version."
	   );
  INFO_MSG("SMA_INFO",
	   "Print this message."
	   );
  INFO_MSG("SMA_SYMMETRIC_SIZE",
	   "Specify the size in bytes of symmetric memory."
	   );
  INFO_MSG("SMA_DEBUG",
	   "Print internal debug information."
	   );
  __shmem_trace(SHMEM_LOG_INFO,
		""
		);
  __shmem_trace(SHMEM_LOG_INFO,
		"We also understand these new environment variables:"
		);
  INFO_MSG("SHMEM_LOG_LEVELS",
	   "Select which kinds of trace messages are enabled"
	   );
  INFO_MSG("SHMEM_LOG_FILE",
	   "Filename to append trace output to, instead of standard error"
	   );
  INFO_MSG("SHMEM_SYMMETRIC_HEAP_SIZE",
	   "Specify the size of symmetric memory."
	   );
  INFO_MSG("SHMEM_BARRIER_ALL_ALGORITHM",
	   "Choose shmem_barrier_all implementation."
	   );
  INFO_MSG("SHMEM_BARRIER_ALGORITHM",
	   "Choose shmem_barrier implementation."
	   );
  INFO_MSG("SHMEM_PE_ACCESSIBLE_TIMEOUT",
	   "How long to wait for PE accessibility check."
	   );
}

void
__shmem_tracers_init(void)
{
  logging_filestream_init();

  sgi_compat_environment_init();

  parse_log_levels();
}

/*
 * spit out which message levels are active
 *
 * OK to use public API here since we're only called when initialized
 *
 */

void
__shmem_tracers_show(void)
{
  if (__shmem_trace_is_enabled(SHMEM_LOG_INIT)) {
    char buf[TRACE_MSG_BUF_SIZE];
    char *p = buf;
    int i;
    trace_table_t *t = tracers;

    strcpy(p, "Enabled Messages: ");

    for (i = 0; i < n_tracers; i += 1) {
      strncat(p, t->text, strlen(t->text));
      strncat(p, " ", 1);
      t += 1;
    }
    __shmem_trace(SHMEM_LOG_INIT,
		  p
		  );
  }
}

void
__shmem_trace(shmem_trace_t msg_type, char *fmt, ...)
{
  if (! __shmem_trace_is_enabled(msg_type)) {
    return;
  }

  {
    char tmp1[TRACE_MSG_BUF_SIZE];
    char tmp2[TRACE_MSG_BUF_SIZE];
    va_list ap;
  
    snprintf(tmp1, TRACE_MSG_BUF_SIZE,
	     "%-8.6f: PE %d: %s: ",
	     __shmem_get_elapsed_clock(),
	     GET_STATE(mype),
	     __level_to_string(msg_type)
	     );
  
    va_start(ap, fmt);
    vsnprintf(tmp2, TRACE_MSG_BUF_SIZE, fmt, ap);
    va_end(ap);
  
    strncat(tmp1, tmp2, TRACE_MSG_BUF_SIZE);
    strncat(tmp1, "\n", TRACE_MSG_BUF_SIZE);
  
    fputs(tmp1, trace_log_stream);
    fflush(trace_log_stream); /* make sure this all goes out in 1 burst */
  }

  if (msg_type == SHMEM_LOG_FATAL) {
    __shmem_exit(1);
  }
}
