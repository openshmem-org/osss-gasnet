#ifndef _GLOBALVAR_H
#define _GLOBALVAR_H 1

/*
 * memory classification and accessibility
 */

extern int    __shmem_symmetric_is_globalvar(void *addr);

extern void   __shmem_symmetric_memory_init(void);
extern void   __shmem_symmetric_memory_finalize(void);
extern void * __shmem_symmetric_var_base(int pe);
extern int    __shmem_symmetric_var_in_range(void *addr, int pe);
extern void * __shmem_symmetric_addr_lookup(void *dest, int pe);

extern int    __shmem_symmetric_addr_accessible(void *addr, int pe);

#endif /* _GLOBALVAR_H */
