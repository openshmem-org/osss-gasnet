/* (c) 2011 University of Houston System.  All rights reserved. */


#include <stdio.h>
#include <sys/param.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#define __USE_BSD 1
#include <unistd.h>

#include <sys/file.h>

#include "trace.h"
#include "state.h"

static const char *self     = "/proc/self/exe";
static const char *self_fmt = "/proc/%ld/exe";

/*
 * inspect our own executable: find the real program name, and open a
 * file descriptor
 */
void
__shmem_executable_init(void)
{
  ssize_t s;
  int fd;

  /* see if the shortcut works */
  s = readlink(self, GET_STATE(exe_name), MAXPATHLEN);

  /* if not, try finding our PID */
  if (s < 0) {
    char buf[MAXPATHLEN];
    snprintf(buf, MAXPATHLEN, self_fmt, getpid());
    s = readlink(buf, GET_STATE(exe_name), MAXPATHLEN);
  }

  /* dunno who I am, complain */
  if (s < 0) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "can't find my own executable name (%s)",
		  strerror(errno)
		  );
    /* NOT REACHED */
  }

  /* bleagh, readlink doesn't null-terminate */
  GET_STATE(exe_name)[s] = '\0';

  /* get a file descriptor */
  fd = open(GET_STATE(exe_name), O_RDONLY, 0);
  if (fd < 0) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "can't open \"%s\" (%s)",
		  GET_STATE(exe_name),
		  strerror(errno)
		  );
    /* NOT REACHED */
  }

  SET_STATE(exe_fd, fd);
}

/*
 * shut down the executable inspection: close the opened file
 * descriptor
 */
void
__shmem_executable_finalize(void)
{
  close(GET_STATE(exe_fd));
}
