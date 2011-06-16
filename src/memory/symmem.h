/* (c) 2011 University of Houston.  All rights reserved. */


#ifndef _SYMMEM_H
#define _SYMMEM_H 1

#include <sys/types.h>

extern void * __shmalloc_no_check(size_t size);
extern int __shmalloc_symmetry_check(size_t size);

#endif /* _SYMMEM_H*/
