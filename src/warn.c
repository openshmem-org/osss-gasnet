#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>

#include "state.h"
#include "warn.h"
#include "updown.h"

typedef struct {
  const int level;
  const char *text;
  int on;
} __warn_table_t;

#define INIT_LEVEL(L, State) { SHMEM_LOG_##L , #L , State }

static
__warn_table_t warnings[] =
  {
    INIT_LEVEL(FATAL,  1),
    INIT_LEVEL(DEBUG,  0),
    INIT_LEVEL(INFO,   0),
    INIT_LEVEL(AUTH,   0),
    INIT_LEVEL(NOTICE, 0)
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
    }
    t += 1;
  }
}

static int
__is_warn_enabled(int level)
{
  int i;
  __warn_table_t *t = warnings;

  for (i = 0; i < n_warnings; i += 1) {
    if (level == t->level) {
      return t->on;
    }
    t += 1;
  }
  return 0;
}

static const char *
__level_to_string(int level)
{
  int i;
  __warn_table_t *t = warnings;

  for (i = 0; i < n_warnings; i += 1) {
    if (level == t->level) {
      return t->text;
    }
    t += 1;
  }
  return "?";
}

void
__shmem_warnings_init(void)
{
  char *shll = getenv("SHMEM_LOGLEVELS");
  if (shll == (char *)NULL) {
    return;
  }

  {
    const char *delims = ",:;";
    char *opt = strtok(shll, delims);

    while (opt != (char *)NULL) {
      __warn_enable(opt);
      opt = strtok((char *)NULL, delims);
    }
  }
}

/* big enough?  I reckon so, we're not writing a novel... */
#define BUF_SIZE 256

void
__shmem_warn(int msg_type, char *fmt, ...)
{
  if (! __is_warn_enabled(msg_type)) {
    return;
  }

  {
    char *prefix_fmt = "SHMEM(PE %d): %s: ";
    char tmp1[BUF_SIZE];
    char tmp2[BUF_SIZE];
    va_list ap;
  
    strncpy(tmp1, prefix_fmt, BUF_SIZE);
  
    snprintf(tmp2, BUF_SIZE, tmp1, __state.mype, __level_to_string(msg_type));
  
    va_start(ap, fmt);
    vsnprintf (tmp1, BUF_SIZE, fmt, ap);
    va_end(ap);
  
    strncat(tmp2, tmp1, BUF_SIZE);
    strncat(tmp2, "\n", BUF_SIZE);
  
    fputs(tmp2, stderr);
    fflush(stderr);		/* make sure this all goes out in 1 burst */
  }

  if (msg_type == SHMEM_LOG_FATAL) {
    __shmem_exit(1);
  }
}
