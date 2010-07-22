/* TODO: pretty messy code, assume no overflow etc */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdarg.h>

#include "state.h"
#include "warn.h"

static int n_warnings_enabled = 0;

typedef struct {
  const int level;
  const char *text;
  int on;
} __warn_table_t;

#define INIT_LEVEL(L) { SHMEM_LOG_##L , #L , 0 }

static
__warn_table_t warnings[] =
  {
    INIT_LEVEL(DEBUG),
    INIT_LEVEL(INFO),
    INIT_LEVEL(NOTICE),
    INIT_LEVEL(AUTH),
    INIT_LEVEL(FATAL)
  };
static const int n_warnings = sizeof(warnings) / sizeof(__warn_table_t);

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

static char *
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
      n_warnings_enabled += 1;
      opt = strtok((char *)NULL, delims);
    }
  }
}

#define BUF_SIZE 1024

void
__shmem_warn(int msg_type, char *fmt, ...)
{
  if (n_warnings_enabled == 0) {
    return;
  }
  if (! __is_warn_enabled(msg_type)) {
    return;
  }

  {
    char *prefix_fmt = "SHMEM(PE %d): %s: ";
    char tmp1[BUF_SIZE];
    char tmp2[BUF_SIZE];
    va_list ap;
  
    strcpy(tmp1, prefix_fmt);
  
    sprintf(tmp2, tmp1, __state.mype, __level_to_string(msg_type));
  
    va_start(ap, fmt);
    vsnprintf (tmp1, BUF_SIZE, fmt, ap);
    va_end(ap);
  
    strcat(tmp2, tmp1);
    strcat(tmp2, "\n");
  
    fputs(tmp2, stderr);
    fflush(stderr);		/* make sure this all goes out in 1 burst */
  }
}
