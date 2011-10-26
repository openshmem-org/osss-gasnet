/*
 *
 * Copyright (c) 2011, University of Houston System and Oak Ridge National
 * Loboratory.
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
 *   National Loboratory nor the names of its contributors may be used to
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



/*
 * Layer to sit on top of the malloc library, exposed to the shmem runtime.
 * This is PE-local and sits just below SHMEM itself.
 */

#include <sys/types.h>

#include "dlmalloc.h"

/*
 * the memory area we manage in this unit.  Not visible to anyone else
 */
static mspace myspace;

/*
 * initialize the memory pool
 */
void
__shmem_mem_init(void *base, size_t capacity)
{
  myspace = create_mspace_with_base(base, capacity, 1);
}

/*
 * clean up memory pool
 */
void
__shmem_mem_finalize(void)
{
  destroy_mspace(myspace);
}

/*
 * return start of pool
 */
void *
__shmem_mem_base(void)
{
  return myspace;
}

/*
 * allocate SIZE bytes from the pool
 */

#define MIN_MALLOC_SIZE 64

void *
__shmem_mem_alloc(size_t size)
{
  return mspace_malloc(myspace, size);
}

/*
 * release memory previously allocated at ADDR
 */
void
__shmem_mem_free(void *addr)
{
  mspace_free(myspace, addr);
}

/*
 * resize ADDR to SIZE bytes
 */
void *
__shmem_mem_realloc(void *addr, size_t size)
{
  return mspace_realloc(myspace, addr, size);
}

/*
 * allocate memory of SIZE bytes, aligning to ALIGNMENT
 */
void *
__shmem_mem_align(size_t alignment, size_t size)
{
  return mspace_memalign(myspace, alignment, size);
}
