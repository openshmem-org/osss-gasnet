#ifndef _WARN_H
#define _WARN_H 1

extern void __shmem_warnings_init(void);

#include <stdarg.h>

extern void __shmem_warn(int msg_type, char *fmt, ...);


#define SHMEM_LOG_FATAL   0	/* unrecoverable problem */

#define SHMEM_LOG_DEBUG   1	/* debugging information */
#define SHMEM_LOG_INFO    2	/* informational */
#define SHMEM_LOG_NOTICE  3	/* serious informational (but not fatal) */
#define SHMEM_LOG_AUTH    4	/* something not authorized */
#define SHMEM_LOG_INIT    5	/* during OpenSHMEM initialization */
#define SHMEM_LOG_MEMORY  6	/* symmetric memory operations */

#endif /* _WARN_H */
