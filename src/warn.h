#ifndef _WARN_H
#define _WARN_H 1

extern void __shmem_warnings_init(void);

#include <stdarg.h>

extern void __shmem_warn(char *type, char *fmt, ...);

#endif /* _WARN_H */
