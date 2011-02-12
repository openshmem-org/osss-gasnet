#ifndef _STATE_H
#define _STATE_H 1

#include <sys/types.h>
#include <sys/utsname.h>

typedef enum {
  PE_UNINITIALIZED=0,		/* start like this */
  PE_UNKNOWN,			/* for when we have no information yet */
  PE_RUNNING,			/* after start_pes() */
  PE_SHUTDOWN,			/* clean exit */
  PE_FAILED,			/* something went wrong */
} pe_status_t;


extern const char * __shmem_state_as_string(pe_status_t s);

typedef struct {
  pe_status_t pe_status;	/* up and running yet?             */
  int numpes;                   /* # of processing elements        */
  int mype;                     /* individual processing element   */
  size_t heapsize;		/* size of symmetric heap (bytes)  */

  int ping_timeout;		/* wait for remote PE to ack ping  */

  struct utsname loc;		/* location information            */

} state_t;

extern state_t __state;

#endif /* _STATE_H */
