#ifndef _WARN_H
#define _WARN_H 1

#include <stdarg.h>
#include <stdio.h>

typedef enum {
  SHMEM_LOG_FATAL=0,		/* unrecoverable problem */

  SHMEM_LOG_ATOMIC,		/* used by various atomic ops */
  SHMEM_LOG_AUTH,		/* something not authorized */
  SHMEM_LOG_BARRIER,		/* barrier ops */
  SHMEM_LOG_BROADCAST,		/* broadcast ops */
  SHMEM_LOG_CACHE,		/* cache flushing ops */
  SHMEM_LOG_COLLECT,		/* [f]collect ops */
  SHMEM_LOG_DEBUG,		/* debugging information */
  SHMEM_LOG_FENCE,		/* fence calls */
  SHMEM_LOG_INFO,		/* informational */
  SHMEM_LOG_INIT,		/* during OpenSHMEM initialization */
  SHMEM_LOG_LOCK,		/* global locks */
  SHMEM_LOG_MEMORY,		/* symmetric memory operations */
  SHMEM_LOG_NOTICE,		/* serious, but non-fatal */
  SHMEM_LOG_QUIET,		/* quiet calls */
  SHMEM_LOG_REDUCE,		/* reduction ops */
  SHMEM_LOG_SERVICE,		/* network service thread */
  SHMEM_LOG_SYMBOLS,		/* dump global dymbol table */
  SHMEM_LOG_VERSION,		/* show library version */
} shmem_trace_t;

extern void __shmem_tracers_init(void);
extern void __shmem_tracers_show(void);

extern void __shmem_maybe_tracers_show_info(void);

extern void __shmem_trace(shmem_trace_t msg_type, char *fmt, ...);
extern int  __shmem_trace_is_enabled(shmem_trace_t level);

#endif /* _WARN_H */
