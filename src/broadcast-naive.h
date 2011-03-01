#ifndef _BROADCAST_NAIVE_H
#define _BROADCAST_NAIVE_H 1

extern void pshmem_broadcast32_naive(void *target, const void *source, size_t nlong,int PE_root, int PE_start, int logPE_stride, int PE_size,long *pSync);
extern void pshmem_broadcast64_naive(void *target, const void *source, size_t nlong,int PE_root, int PE_start, int logPE_stride, int PE_size,long *pSync);

#endif /* _BROADCAST_NAIVE_H */
