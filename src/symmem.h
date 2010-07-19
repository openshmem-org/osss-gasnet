#ifndef _SYMMEM_H
#define _SYMMEM_H 1

#include <sys/types.h>

extern void   __symmetric_memory_init(void);
extern void * __symmetric_var_base(int pe);
extern int    __symmetric_var_in_range(void *addr, int pe);

#endif /* _SYMMEM_H */
