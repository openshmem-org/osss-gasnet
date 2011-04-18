#ifndef _SYMMTEST_H
#define _SYMMTEST_H 1

extern void  __shmem_symmetric_test_with_abort(void *remote_addr,
					       void *local_addr,
					       const char *name,
					       const char *routine);

#endif /* _SYMMTEST_H */
