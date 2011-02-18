#ifndef _WARN_H
#define _WARN_H 1

#include <stdarg.h>
#include <stdio.h>

typedef enum {
  SHMEM_LOG_FATAL=0,		/* unrecoverable problem */
  SHMEM_LOG_DEBUG,		/* debugging information */
  SHMEM_LOG_INFO,		/* informational */
  SHMEM_LOG_NOTICE,		/* serious, but non-fatal */
  SHMEM_LOG_AUTH,		/* something not authorized */
  SHMEM_LOG_INIT,		/* during OpenSHMEM initialization */
  SHMEM_LOG_MEMORY,		/* symmetric memory operations */
  SHMEM_LOG_CACHE,		/* cache flushing ops */
  SHMEM_LOG_BARRIER,		/* barrier ops */
  SHMEM_LOG_BROADCAST,		/* broadcast ops */
  SHMEM_LOG_COLLECT,		/* [f]collect ops */
  SHMEM_LOG_REDUCE,		/* reduction ops */
  SHMEM_LOG_SYMBOLS,		/* dump global dymbol table */
} shmem_trace_t;

extern void __shmem_tracers_init(void);
extern void __shmem_tracers_show(void);

extern void __shmem_trace(shmem_trace_t msg_type, char *fmt, ...);
extern int  __trace_is_enabled(shmem_trace_t level);

#endif /* _WARN_H */
