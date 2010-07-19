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

/*
 * barriers
 */

extern void   shmem_barrier_all(void);

/*
 * symmetric memory management
 */

extern const long SHMEM_MALLOC_OK;
extern const long SHMEM_MALLOC_FAIL;
extern const long SHMEM_MALLOC_ALREADY_FREE;

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
extern const int SHMEM_CMP_EQ;
extern const int SHMEM_CMP_NE;
extern const int SHMEM_CMP_GT;
extern const int SHMEM_CMP_LE;
extern const int SHMEM_CMP_LT;
extern const int SHMEM_CMP_GE;

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

extern short       shmem_short_swap(short *target, short value, int pe);
extern int         shmem_int_swap(int *target, int value, int pe);
extern long        shmem_long_swap(long *target, long value, int pe);
extern long long   shmem_longlong_swap(long long *target, long long value, int pe);
extern float       shmem_float_swap(float *target, float value, int pe);
extern double      shmem_double_swap(double *target, double value, int pe);
extern long        shmem_swap(long *target, long value, int pe);

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
