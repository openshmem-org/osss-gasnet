#ifndef _ATOMIC_H
#define _ATOMIC_H 1

extern void __shmem_atomic_init(void);
extern void __shmem_atomic_finalize(void);

/*
 * choose appropriate memory fence definition
 *
 */

#if defined(__GNUC__)

/* # define LOAD_STORE_FENCE() __asm__ volatile("mfence":::"memory") */
# define LOAD_STORE_FENCE() __sync_synchronize()

#elif defined(__SUNPRO_C)

# include <mbarrier.h>

# define LOAD_STORE_FENCE __machine_rw_barrier()

#else

# error "I don't know how to do memory fences here"

#endif /* choose appropriate memory fence */

#endif /* _ATOMIC_H */
