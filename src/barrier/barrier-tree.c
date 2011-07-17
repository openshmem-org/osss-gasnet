/* (c) 2011 University of Houston System.  All rights reserved. */

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "state.h"
#include "trace.h"

#include "mpp/shmem.h"

/*
 * Tree based barrier generates a binary tree with the PEs in the
 * active set.  
 *
 */

  static void
set_2tree(int PE_start, int PE_stride, int PE_size,
    int *parent, int *child_l, int *child_r, int my_pe)
{
  int max_pe = PE_start + PE_stride * (PE_size - 1);

  *child_l = 2*(my_pe-PE_start) + PE_stride + PE_start;
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

  __shmem_trace(SHMEM_LOG_BARRIER,
      "set2tree: parent = %d, L_child = %d, R_child = %d",
      *parent,
      *child_l,
      *child_r);
}

void
__shmem_barrier_tree(int PE_start, int logPE_stride, int PE_size, long *pSync)

{
  int child_l, child_r, parent;
  const int step = 1 << logPE_stride;
  int my_pe = GET_STATE(mype);
  int thatpe, i;
  int no_children;
  long is_ready,lchild_ready,rchild_ready;


  is_ready = 1;
  lchild_ready = -1;
  rchild_ready = -1;

  shmem_long_wait_until(& pSync[0], SHMEM_CMP_EQ, _SHMEM_SYNC_VALUE);
  shmem_long_wait_until(& pSync[1], SHMEM_CMP_EQ, _SHMEM_SYNC_VALUE);

  pSync[0] = 0;
  pSync[1] = 0;

  /*printf("Tree barrier\n");*/
  set_2tree(PE_start, step, PE_size, &parent,&child_l, &child_r, my_pe);

  no_children  = 0;

  __shmem_trace(SHMEM_LOG_BARRIER,
      "before barrier, R_child = %d L_child = %d",
      child_r,
      child_l
      );

  /* The actual barrier*/

  if(PE_size > 1){
    if (my_pe == PE_start){
      pSync[0] = _SHMEM_SYNC_VALUE;

      if (child_l != -1) {
        shmem_long_get(& lchild_ready,& pSync[0],1, child_l);
        while(lchild_ready != 0)
          shmem_long_get(& lchild_ready,& pSync[0],1, child_l);
        shmem_long_put(& pSync[0],& is_ready, 1, child_l);
        no_children = 1;
      }

      if (child_r != -1) {
        shmem_long_get(& rchild_ready,& pSync[0],1, child_r);
        while(rchild_ready != 0)
          shmem_long_get(& rchild_ready,& pSync[0],1, child_r);
        shmem_long_put(& pSync[0],& is_ready, 1, child_r);
        no_children = 2;
      }

      shmem_long_wait_until(& pSync[1], SHMEM_CMP_EQ, (long)no_children);
      pSync[1] =  _SHMEM_SYNC_VALUE;

    }
    else {
      shmem_long_wait_until(& pSync[0], SHMEM_CMP_EQ, is_ready);

      __shmem_trace(SHMEM_LOG_BARRIER,
          "inside else"
          );

      if (child_l != -1) {
        shmem_long_get(& lchild_ready,& pSync[0],1, child_l);
        while(lchild_ready != 0)
          shmem_long_get(& lchild_ready,& pSync[0],1, child_l);
        shmem_long_put(& pSync[0],& is_ready, 1, child_l);
        no_children = 1;
      }

      if(child_r != -1) {
        shmem_long_get(& rchild_ready,& pSync[0],1, child_r);
        while(rchild_ready != 0)
          shmem_long_get(& rchild_ready,& pSync[0],1, child_r);
        shmem_long_put(& pSync[0],& is_ready, 1, child_r);
        no_children = 2;
      }
      pSync[0] =  _SHMEM_SYNC_VALUE;

      if(no_children == 0){
        pSync[1] =  _SHMEM_SYNC_VALUE;
        shmem_long_inc(& pSync[1], parent);
      }
      else{
        shmem_long_wait_until(& pSync[1], SHMEM_CMP_EQ, (long)no_children);
        pSync[1] =  _SHMEM_SYNC_VALUE;
        shmem_long_inc(& pSync[1], parent);
      }

    }
    __shmem_trace(SHMEM_LOG_BARRIER,
        "at the end of barrier"
        );

  }
}
#include "module_info.h"
module_info_t module_info =
  {
    __shmem_barrier_tree,
    __shmem_barrier_tree,
  };
