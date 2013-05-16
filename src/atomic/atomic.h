/*
 *
 * Copyright (c) 2011 - 2013
 *  University of Houston System and Oak Ridge National Laboratory.
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * o Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * 
 * o Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 
 * o Neither the name of the University of Houston System, Oak Ridge
 *   National Laboratory nor the names of its contributors may be used to
 *   endorse or promote products derived from this software without specific
 *   prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */



#ifndef _ATOMIC_H
#define _ATOMIC_H 1

/*
 * This sub-module's initialization and finalization
 */

extern void __shmem_atomic_init (void);
extern void __shmem_atomic_finalize (void);

/*
 * How do various compilers handle memory barriers and intrinsics?
 *
 * Some guesswork here...
 *
 */

#if defined(__INTEL_COMPILER)
/*
 * icc and friends
 */

# define LOAD_STORE_FENCE __memory_barrier ()
# define SYNC_FETCH_AND_ADD(t, v)  (t) += (v)

#elif defined(__SUNPRO_C)
/*
 * Sun/Oracle Studio
 */

# include <mbarrier.h>

# define LOAD_STORE_FENCE __machine_rw_barrier ()
# define SYNC_FETCH_AND_ADD(t, v)  (t) += (v)

#elif defined(__PGI)
/*
 * Portland Group (PGI)
 */
# include <tmmintrin.h>
# define LOAD_STORE_FENCE _mm_mfence ()
/*
 * found _mm_add_pd but not sure if this is what we want
 */
# define SYNC_FETCH_AND_ADD(t, v)  (t) += (v)

#elif defined(__GNUC__)
/*
 * GCC
 */

# define LOAD_STORE_FENCE __sync_synchronize ()
# define SYNC_FETCH_AND_ADD(t, v) __sync_fetch_and_add(t, v)

#elif defined(__xlc__)
/*
 * IBM XL
 */

# define LOAD_STORE_FENCE __lwsync ()
# define SYNC_FETCH_AND_ADD(t, v)  (t) += (v)

#else

# error "I don't know how to do memory fences here"

#endif /* choose appropriate memory fence */

#endif /* _ATOMIC_H */
