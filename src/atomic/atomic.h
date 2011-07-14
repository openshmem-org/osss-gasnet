/* (c) 2011 University of Houston System.  All rights reserved. */


#ifndef _ATOMIC_H
#define _ATOMIC_H 1

extern void __shmem_atomic_init(void);
extern void __shmem_atomic_finalize(void);

#if defined(__GNUC__)

/* # define LOAD_STORE_FENCE() __asm__ volatile("mfence":::"memory") */
# define LOAD_STORE_FENCE() __sync_synchronize()
# define SYNC_FETCH_AND_ADD(t, v) __sync_fetch_and_add(t, v)

#elif defined(__SUNPRO_C)

# include <mbarrier.h>

# define LOAD_STORE_FENCE __machine_rw_barrier()
# define SYNC_FETCH_AND_ADD(t, v)  (t) += (v)

#else

# error "I don't know how to do memory fences here"

#endif /* choose appropriate memory fence */

#endif /* _ATOMIC_H */
