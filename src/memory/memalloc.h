/* (c) 2011 University of Houston.  All rights reserved. */


#ifndef _MEMALLOC_H
#define _MEMALLOC_H 1

#include <sys/types.h>

extern void   __shmem_mem_init(void *base, size_t capacity);
extern void   __shmem_mem_finalize(void);
extern void * __shmem_mem_base(void);
extern void * __shmem_mem_alloc(size_t size);
extern void   __shmem_mem_free(void *addr);
extern void * __shmem_mem_realloc(void *addr, size_t size);
extern void * __shmem_mem_align(size_t alignment, size_t size);

#endif /* _MEMALLOC_H */
