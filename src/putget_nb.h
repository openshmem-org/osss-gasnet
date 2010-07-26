#ifndef _PUTGET_NB_H
#define _PUTGET_NB_H 1

#include "comms.h"

extern void __shmem_short_put_nb(short *target, short *source,
				 size_t len, int pe,
				 shmem_handle_t *h);
extern void __shmem_int_put_nb(int *target, int *source,
			       size_t len, int pe,
			       shmem_handle_t *h);
extern void __shmem_long_put_nb(long *target, long *source,
				size_t len, int pe,
				shmem_handle_t *h);
extern void __shmem_putmem_nb(long *target, long *source,
			      size_t len, int pe,
			      shmem_handle_t *h);
extern void __shmem_longdouble_put_nb(long double *target,
				      long double *source,
				      size_t len, int pe,
				      shmem_handle_t *h);
extern void __shmem_longlong_put_nb(long long *target,
				    long long *source,
				    size_t len, int pe,
				    shmem_handle_t *h);
extern void __shmem_double_put_nb(double *target, double *source,
				  size_t len, int pe,
				  shmem_handle_t *h);
extern void __shmem_float_put_nb(float *target, float *source,
				 size_t len, int pe,
				 shmem_handle_t *h);

extern void __shmem_wait_nb(shmem_handle_t h);

#endif /* _PUTGET_NB_H */
