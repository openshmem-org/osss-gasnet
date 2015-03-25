/*
 *
 * Copyright (c) 2011 - 2015
 *   University of Houston System and Oak Ridge National Laboratory.
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


#include <stdio.h>
#include <string.h>
#include <math.h>
#include "state.h"
#include "trace.h"
#include "shmem.h"

/*
 * Tree based broadcast generates a binary tree with the PEs in the
 * active with PE_root as the root.  The puts happen in a top down
 * approach.
 *
 */
static inline void
set_2tree (int PE_start, int PE_stride, int PE_size,
           int *parent, int *child_l, int *child_r, int my_pe)
{
    int max_pe = PE_start + PE_stride * (PE_size - 1);
    *child_l = 2 * (my_pe - PE_start) + PE_stride + PE_start;
    *child_r = *child_l + PE_stride;
    /* set parent */
    if (my_pe == PE_start) {
        *parent = -1;
    }
    else {
        *parent = (my_pe - PE_start - PE_stride) / 2;
        *parent -= *parent % PE_stride;
        *parent += PE_start;
    }
    if (*child_l > max_pe) {
        *child_l = *child_r = -1;
    }
    else if (*child_r > max_pe) {
        *child_r = -1;
    }
    shmemi_trace (SHMEM_LOG_BROADCAST,
                  "set2tree: parent = %d, L_child = %d, R_child = %d",
                  *parent, *child_l, *child_r);
}

static inline void
build_tree (int PE_start, int step, int PE_root, int PE_size,
            int *parent, int *child_l, int *child_r, int my_pe)
{
    if (PE_root != 0) {
        int PE_root_abs;
        int other_parent, other_child_l, other_child_r;
        PE_root_abs = PE_start + step * PE_root;
        if (GET_STATE (mype) == PE_start) {
            set_2tree (PE_start, step, PE_size,
                       &other_parent, &other_child_l, &other_child_r,
                       PE_root_abs);
            if (other_parent == GET_STATE (mype)) {
                *parent = PE_root_abs;
            }
            else {
                *parent = other_parent;
            }
            *child_l = other_child_l;
            *child_r = other_child_r;
        }
        else if (GET_STATE (mype) == PE_root_abs) {
            set_2tree (PE_start, step, PE_size,
                       &other_parent, &other_child_l, &other_child_r, PE_start);
            /* other_parent should be -1 */
            *parent = other_parent;
            if (other_child_l == GET_STATE (mype)) {
                *child_l = PE_start;
                *child_r = other_child_r;
            }
            /* if we are the right child of the PE_start */
            else if (other_child_r == GET_STATE (mype)) {
                *child_l = other_child_l;
                *child_r = PE_start;
            }
            else {
                *child_l = other_child_l;
                *child_r = other_child_r;
            }
        }
        /* parent update for children */
        else if (*parent == PE_start) {
            *parent = PE_root_abs;
        }
        else if (*parent == PE_root_abs) {
            *parent = PE_start;
        }
        else {
            ;                   /* TODO: what? */
        }
        /* update potential parents of exchanged nodes */
        if (GET_STATE (mype) != PE_start) {
            if (*child_l == PE_root_abs) {
                *child_l = PE_start;
            }
            else if (*child_r == PE_root_abs) {
                *child_r = PE_start;
            }
        }
    }
}

void
shmemi_broadcast32_tree (void *target, const void *source,
                         size_t nlong,
                         int PE_root, int PE_start,
                         int logPE_stride, int PE_size, long *pSync)
{
    int child_l, child_r, parent;
    const int step = 1 << logPE_stride;
    int my_pe = GET_STATE (mype);
    int *target_ptr, *source_ptr;
    int no_children;
    long is_ready, lchild_ready, rchild_ready;


    is_ready = 1;
    lchild_ready = -1;
    rchild_ready = -1;

    shmem_long_wait_until (&pSync[0], _SHMEM_CMP_EQ, _SHMEM_SYNC_VALUE);
    shmem_long_wait_until (&pSync[1], _SHMEM_CMP_EQ, _SHMEM_SYNC_VALUE);

    pSync[0] = 0;
    pSync[1] = 0;

    target_ptr = (int *) target;
    source_ptr = (int *) source;

    set_2tree (PE_start, step, PE_size, &parent, &child_l, &child_r, my_pe);
    no_children = 0;
    build_tree (PE_start, step, PE_root, PE_size,
                &parent, &child_l, &child_r, my_pe);
    shmemi_trace (SHMEM_LOG_BROADCAST,
                  "before broadcast, R_child = %d L_child = %d",
                  child_r, child_l);
    /* The actual broadcast */

    if (PE_size > 1) {
        if (my_pe == (PE_start + step * PE_root)) {
            pSync[0] = _SHMEM_SYNC_VALUE;

            if (child_l != -1) {
                shmem_long_get (&lchild_ready, &pSync[0], 1, child_l);
                while (lchild_ready != 0)
                    shmem_long_get (&lchild_ready, &pSync[0], 1, child_l);

                shmem_int_put (target_ptr, source_ptr, nlong, child_l);
                shmem_fence ();
                shmem_long_put (&pSync[0], &is_ready, 1, child_l);
                no_children = 1;
            }
            if (child_r != -1) {
                shmem_long_get (&rchild_ready, &pSync[0], 1, child_r);
                while (rchild_ready != 0)
                    shmem_long_get (&rchild_ready, &pSync[0], 1, child_r);

                shmem_int_put (target_ptr, source_ptr, nlong, child_r);
                shmem_fence ();
                shmem_long_put (&pSync[0], &is_ready, 1, child_r);
                no_children = 2;
            }

            shmem_long_wait_until (&pSync[1], _SHMEM_CMP_EQ,
                                   (long) no_children);
            pSync[1] = _SHMEM_SYNC_VALUE;

        }
        else {
            shmem_long_wait_until (&pSync[0], _SHMEM_CMP_EQ, is_ready);
            pSync[0] = _SHMEM_SYNC_VALUE;
            shmemi_trace (SHMEM_LOG_BROADCAST, "inside else");
            memcpy (source_ptr, target_ptr, nlong * sizeof (int));
            if (child_l != -1) {
                shmem_long_get (&lchild_ready, &pSync[0], 1, child_l);
                while (lchild_ready != 0)
                    shmem_long_get (&lchild_ready, &pSync[0], 1, child_l);

                shmem_int_put (target_ptr, source_ptr, nlong, child_l);
                shmem_fence ();
                shmem_long_put (&pSync[0], &is_ready, 1, child_l);
                no_children = 1;
            }
            if (child_r != -1) {
                shmem_long_get (&rchild_ready, &pSync[0], 1, child_r);
                while (rchild_ready != 0)
                    shmem_long_get (&rchild_ready, &pSync[0], 1, child_r);

                shmem_int_put (target_ptr, source_ptr, nlong, child_r);
                shmem_fence ();
                shmem_long_put (&pSync[0], &is_ready, 1, child_r);
                no_children = 2;
            }
            pSync[0] = _SHMEM_SYNC_VALUE;

            if (no_children == 0) {
                pSync[1] = _SHMEM_SYNC_VALUE;
                /* TO DO: Is check for parents pSync required? */
                shmem_long_inc (&pSync[1], parent);
            }
            else {
                shmem_long_wait_until (&pSync[1], _SHMEM_CMP_EQ,
                                       (long) no_children);
                pSync[1] = _SHMEM_SYNC_VALUE;
                /* printf("PE %d incrementing child count on PE
                   %d\n",my_pe,parent); */
                shmem_long_inc (&pSync[1], parent);
            }
        }
        shmemi_trace (SHMEM_LOG_BROADCAST, "at the end of bcast32");
        /* shmem_barrier(PE_start, logPE_stride, PE_size, pSync); */
    }
}

void
shmemi_broadcast64_tree (void *target, const void *source, size_t nlong,
                         int PE_root, int PE_start,
                         int logPE_stride, int PE_size, long *pSync)
{
    shmemi_broadcast32_tree (target, source,
                             nlong * 2,
                             PE_root, PE_start, logPE_stride, PE_size, pSync);
}
