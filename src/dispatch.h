#ifndef _DISPATCH_H
#define _DISPATCH_H 1

typedef void (*__shmem_dispatch_t)(void);

extern __shmem_dispatch_t * __shmem_dispatch;

#define SHMEM_BARRIER_ALL_DISPATCH 0


#endif /* _DISPATCH_H */
