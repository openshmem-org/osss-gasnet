#ifndef _STATS_H
#define _STATS_H 1

#ifdef SHMEM_COLLECT_STATS

#include <stdio.h>

extern void __shmem_stats_init(void);
extern void __shmem_stats_report(void);
extern void __shmem_stats_set_stream(FILE *fp);
extern void __shmem_stats_set_filename(char *fn);
extern void __shmem_stats_put(int pe);
extern void __shmem_stats_get(int pe);
extern void __shmem_stats_my_pe();
extern void __shmem_stats_num_pes();
extern void __shmem_stats_nodename();
extern void __shmem_stats_barrier();

#define SHMEM_STATS_INIT()               __shmem_stats_init()
#define SHMEM_STATS_REPORT()             __shmem_stats_report()
#define SHMEM_STATS_CLEANUP()            __shmem_stats_cleanup()
#define SHMEM_STATS_SET_STREAM(fp)       __shmem_stats_set_stream(fp)
#define SHMEM_STATS_SET_FILENAME(fn)     __shmem_stats_set_filename(fn)
#define SHMEM_STATS_PUT(pe)              __shmem_stats_put(pe)
#define SHMEM_STATS_GET(pe)              __shmem_stats_get(pe)
#define SHMEM_STATS_MY_PE()              __shmem_stats_my_pe()
#define SHMEM_STATS_NUM_PES()            __shmem_stats_num_pes()
#define SHMEM_STATS_NODENAME()           __shmem_stats_nodename()
#define SHMEM_STATS_BARRIER()            __shmem_stats_barrier()

#else /* ! SHMEM_COLLECT_STATS */

/*
 * if not doing stats (production) then these are all empty,
 * and therefore will compile out = no overhead
 */
#define SHMEM_STATS_INIT()
#define SHMEM_STATS_REPORT()
#define SHMEM_STATS_CLEANUP()
#define SHMEM_STATS_SET_STREAM(fp)
#define SHMEM_STATS_SET_FILENAME(fn)
#define SHMEM_STATS_PUT(pe)
#define SHMEM_STATS_GET(pe)
#define SHMEM_STATS_MY_PE()
#define SHMEM_STATS_NUM_PES()
#define SHMEM_STATS_NODENAME()
#define SHMEM_STATS_BARRIER()

#endif /* SHMEM_COLLECT_STATS */

#endif /* _STATS_H */
