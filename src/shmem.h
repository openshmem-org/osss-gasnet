#ifndef _SHMEM_H
#define _SHMEM_H 1

#include <sys/types.h>

/*
 * init & query
 */

extern void   shmem_init(void);
extern void   start_pes(int npes);     /* synonym for shmem_init() */

extern int    shmem_my_pe(void);
extern int    _my_pe(void);            /* synonyms */

extern int    shmem_num_pes(void);
extern int    shmem_n_pes(void);
extern int    _num_pes(void);          /* synonyms */

extern char * shmem_hostname(void);    /* might be FQDN */
extern char * shmem_nodename(void);    /* unqualified */

extern char * shmem_version(void);

/*
 * I/O
 */

extern void   shmem_short_put(short *dest, const short *src, size_t len, int pe);
extern void   shmem_int_put(int *dest, const int *src, size_t len, int pe);
extern void   shmem_long_put(long *dest, const long *src, size_t len, int pe);
extern void   shmem_longlong_put(long long *dest, const long long *src, size_t len, int pe);
extern void   shmem_longdouble_put(long double *dest, const long double *src, size_t len, int pe);
extern void   shmem_double_put(double *dest, const double *src, size_t len, int pe);
extern void   shmem_float_put(float *dest, const float *src, size_t len, int pe);
extern void   shmem_putmem(void *dest, const void *src, size_t len, int pe);
extern void   shmem_put32(void *dest, const void *src, size_t len, int pe);
extern void   shmem_put64(void *dest, const void *src, size_t len, int pe);
extern void   shmem_put128(void *dest, const void *src, size_t len, int pe);

extern void   shmem_short_get(short *dest, const short *src, size_t len, int pe);
extern void   shmem_int_get(int *dest, const int *src, size_t len, int pe);
extern void   shmem_long_get(long *dest, const long *src, size_t len, int pe);
extern void   shmem_longlong_get(long long *dest, const long long *src, size_t len, int pe);
extern void   shmem_longdouble_get(long double *dest, const long double *src, size_t len, int pe);
extern void   shmem_double_get(double *dest, const double *src, size_t len, int pe);
extern void   shmem_float_get(float *dest, const float *src, size_t len, int pe);
extern void   shmem_getmem(void *dest, const void *src, size_t len, int pe);
extern void   shmem_get32(void *dest, const void *src, size_t len, int pe);
extern void   shmem_get64(void *dest, const void *src, size_t len, int pe);
extern void   shmem_get128(void *dest, const void *src, size_t len, int pe);

extern void        shmem_char_p(char *addr, char value, int pe);
extern void        shmem_short_p(short *addr, short value, int pe);
extern void        shmem_int_p(int *addr, int value, int pe);
extern void        shmem_long_p(long *addr, long value, int pe);
extern void        shmem_longlong_p(long long *addr, long long value, int pe);
extern void        shmem_float_p(float *addr, float value, int pe);
extern void        shmem_double_p(double *addr, double value, int pe);
extern void        shmem_longdouble_p(long double *addr, long double value, int pe);

extern char        shmem_char_g(char *addr, int pe);
extern short       shmem_short_g(short *addr, int pe);
extern int         shmem_int_g(int *addr, int pe);
extern long        shmem_long_g(long *addr, int pe);
extern long long   shmem_longlong_g(long long *addr, int pe);
extern float       shmem_float_g(float *addr, int pe);
extern double      shmem_double_g(double *addr, int pe);
extern long double shmem_longdouble_g(long double *addr, int pe);

/*
 * barriers
 */

extern void   shmem_barrier_all(void);
extern void   shmem_barrier(int PE_start, int logPE_stride, int PE_size,
                            long *pSync);
extern void   shmem_fence(void);

/*
 * symmetric memory management
 */

#define SHMEM_MALLOC_OK                   (0L)
#define SHMEM_MALLOC_FAIL                 (1L)
#define SHMEM_MALLOC_ALREADY_FREE         (2L)
#define SHMEM_MALLOC_MEMALIGN_FAILED      (3L)
#define SHMEM_MALLOC_REALLOC_FAILED       (4L)

extern long   malloc_error;

extern void * shmalloc(size_t size);
extern void   shfree(void *ptr);
extern void * shrealloc(void *ptr, size_t size);
extern void * shmemalign(size_t alignment, size_t size);

/*
 * wait operations
 */

/*
 * values aren't important
 */
#define SHMEM_CMP_EQ 0
#define SHMEM_CMP_NE 1
#define SHMEM_CMP_GT 2
#define SHMEM_CMP_LE 3
#define SHMEM_CMP_LT 4
#define SHMEM_CMP_GE 5

extern void   shmem_short_wait_until(short *ivar, int cmp, short cmp_value);
extern void   shmem_int_wait_until(int *ivar, int cmp, int cmp_value);
extern void   shmem_long_wait_until(long *ivar, int cmp, long cmp_value);
extern void   shmem_longlong_wait_until(long long *ivar, int cmp, long long cmp_value);
extern void   shmem_wait_until(long *ivar, int cmp, long cmp_value);

extern void   shmem_short_wait(short *ivar, short cmp_value);
extern void   shmem_int_wait(int *ivar, int cmp_value);
extern void   shmem_long_wait(long *ivar, long cmp_value);
extern void   shmem_longlong_wait(long long *ivar, long long cmp_value);
extern void   shmem_wait(long *ivar, long cmp_value);

/*
 * atomic swaps
 */

extern int         shmem_int_swap(int *target, int value, int pe);
extern long        shmem_long_swap(long *target, long value, int pe);
extern long long   shmem_longlong_swap(long long *target, long long value, int pe);
extern float       shmem_float_swap(float *target, float value, int pe);
extern double      shmem_double_swap(double *target, double value, int pe);
extern long        shmem_swap(long *target, long value, int pe);

extern int         shmem_int_cswap(int *target, int cond, int value, int pe);
extern long        shmem_long_cswap(long *target, long cond, long value, int pe);
extern long long   shmem_longlong_cswap(long long *target,
					long long cond, long long value, int pe);

/*
 * cache flushing
 */

extern void         shmem_clear_cache_inv(void);
extern void         shmem_set_cache_inv(void);
extern void         shmem_clear_cache_line_inv(void *target);
extern void         shmem_set_cache_line_inv(void *target);
extern void         shmem_udcflush(void);
extern void         shmem_udcflush_line(void *target);

/*
 * reductions
 */

#define _SHMEM_BCAST_SYNC_SIZE 256
#define _SHMEM_SYNC_VALUE 0

extern void shmem_int_and_to_all(int *target, int *source, int nreduce,
                          int PE_start, int logPE_stride, int PE_size,
                          int *pWrk, long *pSync);
extern void shmem_long_and_to_all(long *target, long *source, int nreduce,
                            int PE_start, int logPE_stride, int  PE_size,
                            long  *pWrk, long *pSync);
extern void shmem_longlong_and_to_all(long long *target, long long *source, int nreduce,
                               int PE_start, int logPE_stride, int PE_size,
                               long long *pWrk, long *pSync);
extern void shmem_short_and_to_all(short *target, short *source, int nreduce,
                            int PE_start, int logPE_stride, int PE_size,
                            short  *pWrk, long *pSync);

extern void shmem_int_max_to_all(int *target, int *source, int nreduce,
				 int PE_start, int logPE_stride, int PE_size,
				 int *pWrk, long *pSync);
extern void shmem_long_max_to_all(long *target, long *source, int nreduce,
				  int PE_start, int logPE_stride, int PE_size,
				  long *pWrk, long *pSync);
extern void shmem_longlong_max_to_all(long long *target, long long *source, int nreduce,
				      int PE_start, int logPE_stride, int PE_size,
				      long long *pWrk, long *pSync);
extern void shmem_short_max_to_all(short *target, short *source, int nreduce,
				   int PE_start, int logPE_stride, int PE_size,
				   short *pWrk, long *pSync);
extern void shmem_longdouble_max_to_all(long double *target, long double *source, int nreduce,
					int PE_start, int logPE_stride, int PE_size,
					long double *pWrk, long *pSync);
extern void shmem_float_max_to_all(float *target, float *source, int nreduce,
				   int PE_start, int logPE_stride, int PE_size,
				   float *pWrk, long *pSync);
extern void shmem_double_max_to_all(double *target, double *source, int nreduce,
				    int PE_start, int logPE_stride, int PE_size,
				    double *pWrk, long *pSync);

/*
 * broadcasts
 */

extern void shmem_broadcast32(void *target, const void *source, size_t nlong,
                              int PE_root, int PE_start, int logPE_stride, int PE_size,
                              long *pSync);

extern void shmem_broadcast64(void *target, const void *source, size_t nlong,
                              int PE_root, int PE_start, int logPE_stride, int PE_size,
                              long *pSync);

extern long * shmem_sync_init(void);

/*
 * fixed collects
 */

extern void shmem_fcollect32(void *target, const void *source, size_t nlong,
			     int PE_start, int logPE_stride, int PE_size,
			     long *pSync);
extern void shmem_fcollect64(void *target, const void *source, size_t nlong,
			     int PE_start, int logPE_stride, int PE_size,
			     long *pSync);

/*
 * generalized collects
 */

extern void shmem_collect32(void *target, const void *source, size_t nlong,
			    int PE_start, int logPE_stride, int PE_size,
			    long *pSync);
extern void shmem_collect64(void *target, const void *source, size_t nlong,
			    int PE_start, int logPE_stride, int PE_size,
			    long *pSync);

#endif /* _SHMEM_H */
