#ifndef _STATE_H
#define _STATE_H 1

#include <sys/types.h>
#include <sys/param.h>
#include <sys/utsname.h>
#include <sys/time.h>

/*
 * known PE states
 */

typedef enum {
  PE_UNINITIALIZED=0,		/* start like this */
  PE_UNKNOWN,			/* for when we have no information yet */
  PE_RUNNING,			/* after start_pes() */
  PE_SHUTDOWN,			/* clean exit */
  PE_FAILED,			/* something went wrong */
} pe_status_t;

/*
 * translate PE state to human description
 */

extern const char * __shmem_state_as_string(pe_status_t s);

/*
 * per-PE state structure
 */

typedef struct {
  pe_status_t pe_status;	/* up and running yet? */

  int numpes;                   /* # of processing elements */
  int mype;                     /* rank of this processing element */

  size_t heapsize;		/* size of symmetric heap (bytes) */

  struct itimerval ping_timeout; /* wait for remote PE to ack ping */

  struct utsname loc;  /* location information (currently not used) */

  char exe_name[MAXPATHLEN];	/* real name of executable */
  int exe_fd;			/* file descriptor of executable */

} state_t;

/*
 * the per-PE state
 */

extern state_t __state;

/*
 * set/get state variables
 */

#define SET_STATE(var, val)   ( __state.var = val )
#define GET_STATE(var)        ( __state.var )

#endif /* _STATE_H */
