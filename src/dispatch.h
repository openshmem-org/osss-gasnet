#ifndef _DISPATCH_H
#define _DISPATCH_H 1

#include <stdio.h>

typedef void (* __shmem_dispatch_t)();

typedef struct {
  const char *name;
  __shmem_dispatch_t func;
} barrier_dispatches_t;

#define DISPATCH_NULL (__shmem_dispatch_t) NULL

extern __shmem_dispatch_t __shmem_dispatch[];

#define SHMEM_BARRIER_DISPATCH 0

extern void __barrier_dispatch_init(void);

#endif /* _DISPATCH_H */
