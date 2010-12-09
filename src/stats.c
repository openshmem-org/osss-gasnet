#ifdef SHMEM_COLLECT_STATS

/*
 * TODO: do stats collection better.
 * All these separate counter variables are icky.  Also need to add locks.
 * Should build a nicer framework to record more interesting things.
 */

#include <stdio.h>
#include <stdlib.h>

#include "state.h"
#include "warn.h"

static long *put_count;
static long *get_count;
static long *my_pe_count;
static long *num_pes_count;
static long *nodename_count;
static long *barrier_all_count;

static FILE *stats_fp = stderr;

/* calloc zeroes for us */
#define ALLOC_COUNTERS(Op) \
{ \
  Op##_count = (long *) calloc( __state.numpes, sizeof(* Op##_count) ); \
  if (Op##_count == (long *)NULL) {					\
    _shmem_warn(SHMEM_LOG_FATAL,					\
		"couldn't allocate memory for statistics"		\
		);							\
}

#define DISPLAY_COUNTERS(Op) \
{ \
  int i; \
  for (i = 0; i < __state.numpes; i += 1) { \
    fprintf(stats_fp, \
            "SHMEM(PE %d): Counter: %s[%d] = %d\n", \
            __state.mype, \
            #Op, \
            i, \
            Op##_count[i]); \
  } \
}

void
__shmem_stats_init(void)
{
  ALLOC_COUNTERS(put);
  ALLOC_COUNTERS(get);
  ALLOC_COUNTERS(my_pe);
  ALLOC_COUNTERS(num_pes);
  ALLOC_COUNTERS(nodename);
  ALLOC_COUNTERS(barrier_all);
}

void
__shmem_stats_report(void)
{
  DISPLAY_COUNTERS(put);
  DISPLAY_COUNTERS(get);
  DISPLAY_COUNTERS(my_pe);
  DISPLAY_COUNTERS(num_pes);
  DISPLAY_COUNTERS(nodename);
  DISPLAY_COUNTERS(barrier_all);
}

void
__shmem_stats_cleanup(void)
{
  /* what should we do here? */
}

void
__shmem_stats_set_stream(FILE *fp)
{

void
__shmem_stats_set_stream(FILE *fp)
{
  stats_fp = fp;
}

void
__shmem_stats_set_filename(char *fn, char *mode)
{
  char *m = (mode != (char *)NULL) ? mode : "w";
  FILE *fp = fopen(fn, m);
  __shmem_stats_set_stream(fp);
}

void
__shmem_stats_put(int pe)
{
#ifdef SHMEM_STATS_VERBOSE
  fprintf(stats_fp,
          "SHMEM(PE %d): \"%s\" PE %d to PE %d\n",
          __state.mype,
          "put",
          pe
         );
#endif /* SHMEM_STATS_VERBOSE */
  put_count[__state.mype] += 1;
}

void
__shmem_stats_get(int pe)
{
#ifdef SHMEM_STATS_VERBOSE
  fprintf(stats_fp,
          "SHMEM(PE %d): \"%s\" PE %d from PE %d\n",
         __state.mype,
          "get",
          pe
         );
#endif /* SHMEM_STATS_VERBOSE */
  get_count[__state.mype] += 1;
}

void
__shmem_stats_my_pe(void)
{
#ifdef SHMEM_STATS_VERBOSE
  fprintf(stats_fp,
          "SHMEM(PE %d): \"%s\" on PE %d\n",
          __state.mype,
          "my_pe"
         );
#endif /* SHMEM_STATS_VERBOSE */
  my_pe_count[__state.mype] += 1;
}

void
__shmem_stats_num_pes(void)
{
#ifdef SHMEM_STATS_VERBOSE
  fprintf(stats_fp,
          "SHMEM(PE %d): \"%s\" on PE %d\n",
          __state.mype,
          "num_pes"
         );
#endif /* SHMEM_STATS_VERBOSE */
  num_pes_count[__state.mype] += 1;
}

void
__shmem_stats_nodename(void)
{
#ifdef SHMEM_STATS_VERBOSE
  fprintf(stats_fp,
          "SHMEM(PE %d): \"%s\" on PE %d\n",
          __state.mype,
          "nodename"
         );
#endif /* SHMEM_STATS_VERBOSE */
  nodename_count[__state.mype] += 1;
}

void
__shmem_stats_barrier_all(void)
{
#ifdef SHMEM_STATS_VERBOSE
  fprintf(stats_fp,
          "SHMEM(PE %d): \"%s\"\n",
          __state.mype,
          "barrier_all"
         );
#endif /* SHMEM_STATS_VERBOSE */
  barrier_all_count[__state.mype] += 1;
}

#endif /* SHMEM_COLLECT_STATS */
