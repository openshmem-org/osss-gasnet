#ifndef _WARN_H
#define _WARN_H 1

extern void __shmem_warnings_init(void);

#include <stdarg.h>

extern void __shmem_warn(int msg_type, char *fmt, ...);


#define SHMEM_LOG_DEBUG   0
#define SHMEM_LOG_INFO    1
#define SHMEM_LOG_NOTICE  2
#define SHMEM_LOG_AUTH    3
#define SHMEM_LOG_FATAL   4

#endif /* _WARN_H */
