#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>

#include "state.h"
#include "warn.h"
#include "updown.h"
#include "comms.h"

typedef struct {
  const shmem_warn_t level;
  const char *text;
  int on;
} __warn_table_t;

#define INIT_LEVEL(L, State) { SHMEM_LOG_##L , #L , State }

static const char *shmem_loglevels_envvar = "SHMEM_LOG_LEVELS";

static
__warn_table_t warnings[] =
  {
    INIT_LEVEL(FATAL,      1),

    INIT_LEVEL(DEBUG,  	   0),
    INIT_LEVEL(INFO,   	   0),
    INIT_LEVEL(AUTH,   	   0),
    INIT_LEVEL(NOTICE, 	   0),
    INIT_LEVEL(INIT,   	   0),
    INIT_LEVEL(MEMORY, 	   0),
    INIT_LEVEL(CACHE,  	   0),
    INIT_LEVEL(BARRIER,    0),
    INIT_LEVEL(COLLECT,    0),
    INIT_LEVEL(REDUCE,     0)
  };
static const int n_warnings = sizeof(warnings) / sizeof(warnings[0]);

static void
__warn_enable(char *w)
{
  int i;
  __warn_table_t *t = warnings;

  for (i = 0; i < n_warnings; i += 1) {
    if (strcasecmp(w, t->text) == 0) {
      t->on = 1;
      return;
      /* NOT REACHED */
    }
    t += 1;
  }
}

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
      return t->on;
      /* NOT REACHED */
    }
    t += 1;
  }
  return 0;
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
      __warn_enable(opt);
      opt = strtok((char *) NULL, delims);
    }
  }
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
  
    fputs(tmp1, stderr);
    fflush(stderr);		/* make sure this all goes out in 1 burst */
  }

  if (msg_type == SHMEM_LOG_FATAL) {
    __shmem_exit(1);
  }
}
