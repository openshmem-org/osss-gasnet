
#ifndef _PUTGET_H
#define _PUTGET_H 1

extern void __shmem_short_put_nbi(short *target, short *source, size_t len, int pe);
extern void __shmem_int_put_nbi(int *target, int *source, size_t len, int pe);
extern void __shmem_long_put_nbi(long *target, long *source, size_t len, int pe);
extern void __shmem_longdouble_put_nbi(long double *target, long double *source, size_t len, int pe);
extern void __shmem_longlong_put_nbi(long long *target, long long *source, size_t len, int pe);
extern void __shmem_double_put_nbi(double *target, double *source, size_t len, int pe);
extern void __shmem_float_put_nbi(float *target, float *source, size_t len, int pe);

extern void __shmem_wait_syncnbi_puts(void);

extern void  symmetric_test_with_abort(void *remote_addr,
				       void *local_addr,
				       const char *name,
				       const char *routine);

#endif /* _PUTGET_H */
