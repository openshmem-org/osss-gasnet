#ifndef _COMMS_H
#define _COMMS_H 1

#include <sys/types.h>

extern void __comms_init(void);

extern void __comms_exit(int status);
extern void __comms_shutdown(int status);

extern int  __comms_mynode(void);
extern int  __comms_nodes(void);

extern char * __comms_getenv(const char *name);

typedef enum {
  SHMEM_COMMS_SPINBLOCK=0,
  SHMEM_COMMS_SPIN,
  SHMEM_COMMS_BLOCK
} comms_spinmode_t;

extern void __comms_set_waitmode(comms_spinmode_t mode);

extern void __comms_put(void *dst, void *src, size_t len, int pe);
extern void __comms_get(void *dst, void *src, size_t len, int pe);
extern void __comms_put_val(void *dst, long src, size_t len, int pe);
extern long __comms_get_val(void *src, size_t len, int pe);

extern void * __comms_short_put_nb(short *target, short *source,
				   size_t len, int pe);
extern void * __comms_int_put_nb(int *target, int *source,
				 size_t len, int pe);
extern void * __comms_long_put_nb(long *target, long *source,
				  size_t len, int pe);
extern void * __comms_putmem_nb(long *target, long *source,
				size_t len, int pe);
extern void * __comms_longdouble_put_nb(long double *target,
					long double *source,
					size_t len, int pe);
extern void * __comms_longlong_put_nb(long long *target,
				      long long *source,
				      size_t len, int pe);
extern void * __comms_double_put_nb(double *target, double *source,
				    size_t len, int pe);
extern void * __comms_float_put_nb(float *target, float *source,
				   size_t len, int pe);
extern void   __comms_wait_nb(void *h);

extern void   __comms_fence(void);

extern int    __comms_is_globalvar(void *addr);

extern void   __symmetric_memory_init(void);
extern void   __symmetric_memory_finalize(void);
extern void * __symmetric_var_base(int pe);
extern int    __symmetric_var_in_range(void *addr, int pe);
extern void * __symmetric_addr_lookup(void *dest, int pe);

extern int    __comms_addr_accessible(void *addr, int pe);

extern void __comms_barrier_all(void);
extern void __comms_barrier(int PE_start, int logPE_stride,
			    int PE_size, long *pSync);

extern void __comms_poll(void);
extern void __comms_pause(void);

#define WAIT_ON_COMPLETION(p) do { __comms_poll(); } while (! (p))

extern void  __comms_swap_request(void *target, void *value, size_t nbytes,
				  int pe,
				  void *retval);

extern void  __comms_cswap_request(void *target,
				   void *cond, void *value, size_t nbytes,
				   int pe,
				   void *retval);

extern void  __comms_fadd_request(void *target, void *value, size_t nbytes,
				  int pe,
				  void *retval);

extern void  __comms_finc_request(void *target, size_t nbytes,
				  int pe,
				  void *retval);

extern void  __comms_add_request(void *target, void *value, size_t nbytes,
				 int pe);

extern void  __comms_inc_request(void *target, size_t nbytes,
				 int pe);

extern int   __comms_ping_request(int pe);

extern void  __comms_quiet_request(void);
extern void  __comms_fence_request(void);

#endif /* _COMMS_H */
