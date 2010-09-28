#ifndef _STATE_H
#define _STATE_H 1

#include <sys/types.h>
#include <sys/utsname.h>

typedef struct __state {
  int initialized;              /* up and running yet?             */
  int numpes;                   /* # of processing elements        */
  int mype;                     /* individual processing element   */
  size_t heapsize;		/* size of symmetric heap (bytes)  */

  struct utsname loc;		/* location information            */

} state_t;

extern state_t __state;

#endif /* _STATE_H */
