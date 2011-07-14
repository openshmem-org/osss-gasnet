/* (c) 2011 University of Houston System.  All rights reserved. */


#ifndef _COMMS_H
#define _COMMS_H 1

#include <sys/types.h>

/*
 * initialization and shutdown of the communications layer
 */
extern void __shmem_comms_init(void);
extern void __shmem_comms_finalize(int status);
extern void __shmem_comms_exit(int status);

/*
 * query the environment
 */
extern int  __shmem_comms_mynode(void);
extern int  __shmem_comms_nodes(void);

/*
 * do a getenv (potentially multi-node)
 */
extern char * __shmem_comms_getenv(const char *name);

/*
 * different spin/block modes
 */
typedef enum {
  SHMEM_COMMS_SPINBLOCK=0,
  SHMEM_COMMS_SPIN,
  SHMEM_COMMS_BLOCK
} comms_spinmode_t;

extern void __shmem_comms_set_waitmode(comms_spinmode_t mode);

/*
 * handlers for puts and gets
 */
extern void __shmem_comms_put(void *dst, void *src, size_t len, int pe);
extern void __shmem_comms_get(void *dst, void *src, size_t len, int pe);
extern void __shmem_comms_put_val(void *dst, long src, size_t len, int pe);
extern long __shmem_comms_get_val(void *src, size_t len, int pe);

/*
 * handlers for non-blocking puts and gets (FUTURE)
 */
extern void * __shmem_comms_short_put_nb(short *target, const short *source,
				   size_t len, int pe);
extern void * __shmem_comms_int_put_nb(int *target, const int *source,
				 size_t len, int pe);
extern void * __shmem_comms_long_put_nb(long *target, const long *source,
				  size_t len, int pe);
extern void * __shmem_comms_putmem_nb(long *target, const long *source,
				size_t len, int pe);
extern void * __shmem_comms_longdouble_put_nb(long double *target,
					const long double *source,
					size_t len, int pe);
extern void * __shmem_comms_longlong_put_nb(long long *target,
				      const long long *source,
				      size_t len, int pe);
extern void * __shmem_comms_double_put_nb(double *target, const double *source,
				    size_t len, int pe);
extern void * __shmem_comms_float_put_nb(float *target, const float *source,
				   size_t len, int pe);

extern void * __shmem_comms_short_get_nb(short *target, const short *source,
				   size_t len, int pe);
extern void * __shmem_comms_int_get_nb(int *target, const int *source,
				 size_t len, int pe);
extern void * __shmem_comms_long_get_nb(long *target, const long *source,
				  size_t len, int pe);
extern void * __shmem_comms_getmem_nb(long *target, const long *source,
				size_t len, int pe);
extern void * __shmem_comms_longdouble_get_nb(long double *target,
					const long double *source,
					size_t len, int pe);
extern void * __shmem_comms_longlong_get_nb(long long *target,
				      const long long *source,
				      size_t len, int pe);
extern void * __shmem_comms_double_get_nb(double *target, const double *source,
				    size_t len, int pe);
extern void * __shmem_comms_float_get_nb(float *target, const float *source,
				   size_t len, int pe);

extern void   __shmem_comms_wait_nb(void *h);
extern int    __shmem_comms_test_nb(void *h);


/*
 * exported to service thread to control polling and waiting
 */
extern void   __shmem_comms_fence_service(void);
extern void   __shmem_comms_poll_service(void);
extern void   __shmem_comms_pause(void);
extern void   __shmem_comms_set_service_pause(double ms);

/*
 * utility to wait on remote updates
 */
/* #define WAIT_ON_COMPLETION(p) do { __shmem_comms_pause(); } while (! (p)) */
#define WAIT_ON_COMPLETION(p) GASNET_BLOCKUNTIL(p)

/*
 * for accessibility timeouts
 */
extern int   __shmem_comms_ping_request(int pe);

/*
 * barriers
 */
extern void __shmem_comms_barrier_all(void);
extern void __shmem_comms_barrier(int PE_start, int logPE_stride,
				  int PE_size, long *pSync);

/*
 * swaps, other atomics
 */
extern void  __shmem_comms_swap_request(void *target, void *value, size_t nbytes,
				  int pe,
				  void *retval);

extern void  __shmem_comms_cswap_request(void *target,
				   void *cond, void *value, size_t nbytes,
				   int pe,
				   void *retval);

extern void  __shmem_comms_fadd_request(void *target, void *value, size_t nbytes,
				  int pe,
				  void *retval);

extern void  __shmem_comms_finc_request(void *target, size_t nbytes,
				  int pe,
				  void *retval);

extern void  __shmem_comms_add_request(void *target, void *value, size_t nbytes,
				 int pe);

extern void  __shmem_comms_inc_request(void *target, size_t nbytes,
				 int pe);

/*
 * fence and quiet initiators
 */
extern void  __shmem_comms_quiet_request(void);
extern void  __shmem_comms_fence_request(void);

/*
 * TODO: this should be in ../globalvar
 */
extern void * __shmem_symmetric_addr_lookup(void *dest, int pe);

#endif /* _COMMS_H */
