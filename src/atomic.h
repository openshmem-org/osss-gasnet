#ifndef _ATOMIC_H
#define _ATOMIC_H 1

extern void __shmem_atomic_init(void);
extern void __shmem_atomic_finalize(void);

#define LOAD_STORE_FENCE() asm volatile("mfence":::"memory")

#endif /* _ATOMIC_H */
