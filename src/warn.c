#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>

#include "state.h"
#include "warn.h"
#include "updown.h"
#include "comms.h"

typedef enum {
  OFF=0,
  ON,
} __warn_state_t;

typedef struct {
  const shmem_warn_t level;
  const char *text;
  __warn_state_t state;
} __warn_table_t;

#define INIT_LEVEL(L, State) { SHMEM_LOG_##L , #L , State }

static const char *shmem_loglevels_envvar = "SHMEM_LOG_LEVELS";

static const char *shmem_logfile_envvar = "SHMEM_LOG_FILE";

static
__warn_table_t warnings[] =
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
    INIT_LEVEL(COLLECT,    OFF),
    INIT_LEVEL(REDUCE,     OFF),
  };
static const int n_warnings = sizeof(warnings) / sizeof(warnings[0]);

/*
 * enable the named message category.  Return 1 if matched,
 * 0 if not found
 *
 */

static int
__warn_enable_text(char *warning)
{
  int i;
  __warn_table_t *t = warnings;

  for (i = 0; i < n_warnings; i += 1) {
    if (strcasecmp(warning, t->text) == 0) {
      t->state = ON;
      return 1;
      /* NOT REACHED */
    }
    t += 1;
  }
  return 0;
}

/*
 * translate the messag level to text description
 *
 */

static const char *
__level_to_string(shmem_warn_t level)
{
  int i;
  __warn_table_t *t = warnings;

  for (i = 0; i < n_warnings; i += 1) {
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
__warn_is_enabled(shmem_warn_t level)
{
  int i;
  __warn_table_t *t = warnings;

  for (i = 0; i < n_warnings; i += 1) {
    if (level == t->level) {
      return (t->state == ON);
      /* NOT REACHED */
    }
    t += 1;
  }
  return 0;
}

static FILE *warn_log_stream;

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

  warn_log_stream = stderr;

  shlf = __comms_getenv(shmem_logfile_envvar);
  if (shlf == (char *) NULL) {
    return;
  }

  fp = fopen(shlf, "a");
  if (fp == (FILE *) NULL) {
    return;
  }

  warn_log_stream = fp;
}

void
__shmem_warnings_init(void)
{
  char *shll = __comms_getenv(shmem_loglevels_envvar);
  if (shll == (char *) NULL) {
    return;
  }

  {
    const char *delims = ",:;";
    char *opt = strtok(shll, delims);

    while (opt != (char *) NULL) {
      (void) __warn_enable_text(opt);
      opt = strtok((char *) NULL, delims);
    }
  }

  logging_filestream_init();

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
__shmem_warn(shmem_warn_t msg_type, char *fmt, ...)
{
  if (! __warn_is_enabled(msg_type)) {
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
  
    fputs(tmp1, warn_log_stream);
    fflush(warn_log_stream); /* make sure this all goes out in 1 burst */
  }

  if (msg_type == SHMEM_LOG_FATAL) {
    __shmem_exit(1);
  }
}
