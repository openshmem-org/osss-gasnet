#ifndef _STATE_H
#define _STATE_H 1

#include <sys/types.h>

typedef struct __state {
  int initialized;              /* up and running yet?             */
  int numpes;                   /* # of processing elements        */
  int mype;                     /* individual processing element   */
  size_t heapsize;		/* size of symmetric heap (bytes)  */
  char *hostname;               /* possibly qualified host name    */
  char *nodename;               /* unqualified host name           */
} state_t;

extern state_t __state;

#endif /* _STATE_H */
