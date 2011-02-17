#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>

#include "state.h"
#include "trace.h"
#include "updown.h"
#include "comms.h"

static const char *shmem_loglevels_envvar = "SHMEM_LOG_LEVELS";

static const char *shmem_logfile_envvar = "SHMEM_LOG_FILE";

typedef enum {
  OFF=0,
  ON,
} __trace_state_t;		/* tracing states */

typedef struct {
  const shmem_trace_t level;	/* SHMEM_LOG_XXX symbol for logging */
  const char *text;		/* human readable name */
  __trace_state_t state;	/* off or on */
} __trace_table_t;

#define INIT_LEVEL(L, State) { SHMEM_LOG_##L , #L , State }

static
__trace_table_t tracers[] =
  {
    INIT_LEVEL(FATAL,      ON ),

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
  };
static const int n_tracers = sizeof(tracers) / sizeof(tracers[0]);

/*
 * enable the named message category.  Return 1 if matched,
 * 0 if not found
 *
 */

static int
__trace_enable_text(char *trace)
{
  int i;
  __trace_table_t *t = tracers;

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
__trace_enable_all(void)
{
  int i;
  __trace_table_t *t = tracers;

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
  __trace_table_t *t = tracers;

  for (i = 0; i < n_tracers; i += 1) {
    if (level == t->level) {
      return t->text;
      /* NOT REACHED */
    }
    t += 1;
  }
  return "?";
}

/*
 * spit out which message levels are active
 *
 * OK to use public API here since we're only called when initialized
 *
 */

static void
maybe_show_trace_levels(void)
{
  char buf[256];
  char *p = buf;
  int i;
  __trace_table_t *t = tracers;

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

/* -- end of static -- */

int
__trace_is_enabled(shmem_trace_t level)
{
  int i;
  __trace_table_t *t = tracers;

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

  shlf = __comms_getenv(shmem_logfile_envvar);
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

void
__shmem_tracers_init(void)
{
  char *shll = __comms_getenv(shmem_loglevels_envvar);

  logging_filestream_init();

  if (shll != (char *) NULL) {

    const char *delims = ",:;";
    char *opt = strtok(shll, delims);

    while (opt != (char *) NULL) {
      if (strcasecmp(opt, "all") == 0) {
	__trace_enable_all();
	break;
	/* NOT REACHED */
      }
      (void) __trace_enable_text(opt);
      opt = strtok((char *) NULL, delims);
    }

  }

  maybe_show_trace_levels();
    
}

/*
 * big enough?  I reckon so, we're not writing a novel...
 *
 * TODO: alternatively, can loop on increasing buffer size on
 * overflow...better way to do it?  Or is it worth it?
 *
 */

#define BUF_SIZE 256

void
__shmem_trace(shmem_trace_t msg_type, char *fmt, ...)
{
  if (! __trace_is_enabled(msg_type)) {
    return;
  }

  {
    char tmp1[BUF_SIZE];
    char tmp2[BUF_SIZE];
    va_list ap;
  
    snprintf(tmp1, BUF_SIZE,
	     "SHMEM(PE %d): %s: ",
	     __state.mype, __level_to_string(msg_type)
	     );
  
    va_start(ap, fmt);
    vsnprintf(tmp2, BUF_SIZE, fmt, ap);
    va_end(ap);
  
    strncat(tmp1, tmp2, BUF_SIZE);
    strncat(tmp1, "\n", BUF_SIZE);
  
    fputs(tmp1, trace_log_stream);
    fflush(trace_log_stream); /* make sure this all goes out in 1 burst */
  }

  if (msg_type == SHMEM_LOG_FATAL) {
    __shmem_exit(1);
  }
}
