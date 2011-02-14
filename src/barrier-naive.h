#ifndef _BARRIER_NAIVE_H
#define _BARRIER_NAIVE_H 1

extern void pshmem_barrier_all_naive(void);

extern void pshmem_barrier_naive(int PE_start, int logPE_stride, int PE_size,
				 long *pSync);

#endif /* _BARRIER_NAIVE_H */
