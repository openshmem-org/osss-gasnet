/* TODO: pretty messy code, assume no overflow etc */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "state.h"

#define BUF_SIZE 1024

void
__shmem_warn(char *msg_type, char *fmt, ...)
{
  char *prefix_fmt = "SHMEM(PE %d): %s: ";
  char tmp1[BUF_SIZE];
  char tmp2[BUF_SIZE];
  va_list ap;

  strcpy(tmp1, prefix_fmt);

  sprintf(tmp2, tmp1, __state.mype, msg_type);

  va_start(ap, fmt);
  vsnprintf (tmp1, BUF_SIZE, fmt, ap);
  va_end(ap);

  strcat(tmp2, tmp1);
  strcat(tmp2, "\n");

  fputs(tmp2, stderr);
  fflush(stderr);
}
