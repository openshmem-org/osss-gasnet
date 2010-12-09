#ifndef _WARN_H
#define _WARN_H 1

extern void __shmem_warnings_init(void);

#include <stdarg.h>

typedef enum {
  SHMEM_LOG_FATAL=0,		/* unrecoverable problem */
  SHMEM_LOG_DEBUG,		/* debugging information */
  SHMEM_LOG_INFO,		/* informational */
  SHMEM_LOG_NOTICE,		/* serious, but non-fatal */
  SHMEM_LOG_AUTH,		/* something not authorized */
  SHMEM_LOG_INIT,		/* during OpenSHMEM initialization */
  SHMEM_LOG_MEMORY,		/* symmetric memory operations */
  SHMEM_LOG_CACHE		/* cache flushing ops */
} shmem_warn_t;

extern void __shmem_warn(shmem_warn_t msg_type, char *fmt, ...);
extern int  __warn_is_enabled(shmem_warn_t level);

#endif /* _WARN_H */
