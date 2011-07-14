/* (c) 2011 University of Houston System.  All rights reserved. */


#ifndef _SYMMTEST_H
#define _SYMMTEST_H 1

extern void  __shmem_symmetric_test_with_abort(void *remote_addr,
					       void *local_addr,
					       const char *name,
					       const char *routine);

extern void * __shmem_symmetric_addr_lookup(void *dest, int pe);
    
extern int    __shmem_symmetric_addr_accessible(void *addr, int pe);

extern int    __shmem_is_symmetric(void *addr);

#endif /* _SYMMTEST_H */
