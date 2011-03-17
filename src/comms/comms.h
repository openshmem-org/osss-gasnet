#ifndef _COMMS_H
#define _COMMS_H 1

#include <sys/types.h>

extern void __shmem_comms_init(void);
extern void __shmem_comms_finalize(int status);
extern void __shmem_comms_exit(int status);

extern int  __shmem_comms_mynode(void);
extern int  __shmem_comms_nodes(void);

extern char * __shmem_comms_getenv(const char *name);

typedef enum {
  SHMEM_COMMS_SPINBLOCK=0,
  SHMEM_COMMS_SPIN,
  SHMEM_COMMS_BLOCK
} comms_spinmode_t;

extern void __shmem_comms_set_waitmode(comms_spinmode_t mode);

extern void __shmem_comms_put(void *dst, void *src, size_t len, int pe);
extern void __shmem_comms_get(void *dst, void *src, size_t len, int pe);
extern void __shmem_comms_put_val(void *dst, long src, size_t len, int pe);
extern long __shmem_comms_get_val(void *src, size_t len, int pe);

extern void * __shmem_comms_short_put_nb(short *target, short *source,
				   size_t len, int pe);
extern void * __shmem_comms_int_put_nb(int *target, int *source,
				 size_t len, int pe);
extern void * __shmem_comms_long_put_nb(long *target, long *source,
				  size_t len, int pe);
extern void * __shmem_comms_putmem_nb(long *target, long *source,
				size_t len, int pe);
extern void * __shmem_comms_longdouble_put_nb(long double *target,
					long double *source,
					size_t len, int pe);
extern void * __shmem_comms_longlong_put_nb(long long *target,
				      long long *source,
				      size_t len, int pe);
extern void * __shmem_comms_double_put_nb(double *target, double *source,
				    size_t len, int pe);
extern void * __shmem_comms_float_put_nb(float *target, float *source,
				   size_t len, int pe);
extern void   __shmem_comms_wait_nb(void *h);

extern void   __shmem_comms_fence(void);

extern int    __shmem_comms_is_globalvar(void *addr);

extern void   __shmem_symmetric_memory_init(void);
extern void   __shmem_symmetric_memory_finalize(void);
extern void * __shmem_symmetric_var_base(int pe);
extern int    __shmem_symmetric_var_in_range(void *addr, int pe);
extern void * __shmem_symmetric_addr_lookup(void *dest, int pe);

extern int    __shmem_comms_addr_accessible(void *addr, int pe);

extern void __shmem_comms_barrier_all(void);
extern void __shmem_comms_barrier(int PE_start, int logPE_stride,
			    int PE_size, long *pSync);

extern void __shmem_comms_poll(void);
extern void __shmem_comms_pause(void);

#define WAIT_ON_COMPLETION(p) while (! (p)) { __shmem_comms_pause(); }

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

extern int   __shmem_comms_ping_request(int pe);

extern void  __shmem_comms_quiet_request(void);
extern void  __shmem_comms_fence_request(void);

#endif /* _COMMS_H */
