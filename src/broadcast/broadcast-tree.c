/* (c) 2011 University of Houston System.  All rights reserved. */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "state.h"
#include "trace.h"
#include "mpp/shmem.h"
/*
 * Tree based broadcast generates a binary tree with the PEs in the
 * active with PE_root as the root.  The puts happen in a top down
 * approach.
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
  __shmem_trace(SHMEM_LOG_BROADCAST,
      "set2tree: parent = %d, L_child = %d, R_child = %d",
      *parent,
      *child_l,
      *child_r);
}
  static void
build_tree(int PE_start, int step, int PE_root, int PE_size,
    int *parent, int *child_l, int *child_r, int my_pe)
{ 
  int inter;
  if (PE_root != 0) {
    int PE_root_abs;
    int other_parent, other_child_l, other_child_r;
    PE_root_abs = PE_start + step * PE_root;
    if (GET_STATE(mype) == PE_start) {
      set_2tree(PE_start, step, PE_size,
          &other_parent, &other_child_l, &other_child_r,
          PE_root_abs);
      if (other_parent == GET_STATE(mype)) {
        *parent = PE_root_abs;
      }
      else {
        *parent = other_parent;
      }
      *child_l = other_child_l;
      *child_r = other_child_r;
      inter = PE_root_abs + 1;
    }
    else if (GET_STATE(mype) == PE_root_abs) {
      set_2tree(PE_start, step, PE_size,
          &other_parent, &other_child_l, &other_child_r,
          PE_start);
      /* other_parent should be -1 */
      *parent = other_parent;
      if (other_child_l ==  GET_STATE(mype)) {
        *child_l = PE_start;
        *child_r = other_child_r;
      }
      /* if we are the right
       * child of the PE_start
       * */
      else if (other_child_r ==  GET_STATE(mype)) {
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
      ; /* TODO: what? */
    }
    /* update potential parents of
     * exchanged nodes */
    if (GET_STATE(mype) != PE_start) {
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
__shmem_broadcast32_tree(void *target, const void *source,
    size_t nlong,
    int PE_root, int PE_start,
    int logPE_stride, int PE_size,
    long *pSync)        
{
  int child_l, child_r, parent;           
  const int step = 1 << logPE_stride;             
  int my_pe = GET_STATE(mype);
  int *target_ptr, *source_ptr;
  int old_target;
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

  target_ptr = (int *) target;
  source_ptr = (int *) source;

  set_2tree(PE_start, step, PE_size, &parent,&child_l, &child_r, my_pe);
  old_target = *target_ptr;
  no_children  = 0;
  build_tree(PE_start, step, PE_root, PE_size,
      &parent, &child_l, &child_r,
      my_pe);
  __shmem_trace(SHMEM_LOG_BROADCAST,
      "before broadcast, R_child = %d L_child = %d",
      child_r,
      child_l
      );
  /* The actual broadcast*/ 

  if(PE_size > 1){
    if (my_pe == (PE_start + step * PE_root)) {
      pSync[0] = _SHMEM_SYNC_VALUE;

      if (child_l != -1) {
        shmem_long_get(& lchild_ready,& pSync[0],1, child_l);
        while(lchild_ready != 0)
          shmem_long_get(& lchild_ready,& pSync[0],1, child_l);

        shmem_int_put(target_ptr, source_ptr, nlong, child_l);
        shmem_fence();  
        shmem_long_put(& pSync[0],& is_ready, 1, child_l);  
        no_children = 1;
      }
      if (child_r != -1) {
        shmem_long_get(& rchild_ready,& pSync[0],1, child_r);
        while(rchild_ready != 0)
          shmem_long_get(& rchild_ready,& pSync[0],1, child_r);

        shmem_int_put(target_ptr, source_ptr, nlong, child_r);  
        shmem_fence();  
        shmem_long_put(& pSync[0],& is_ready, 1, child_r);
        no_children = 2;    
      }

      shmem_long_wait_until(& pSync[1], SHMEM_CMP_EQ, (long)no_children);
      pSync[1] =  _SHMEM_SYNC_VALUE;

    }
    else {
      shmem_long_wait_until(& pSync[0], SHMEM_CMP_EQ, is_ready);
      pSync[0] =  _SHMEM_SYNC_VALUE;
      __shmem_trace(SHMEM_LOG_BROADCAST,
          "inside else"
          );
      memcpy(source_ptr,target_ptr, nlong* sizeof(int));
      /*LOAD_STORE_FENCE();*/
      if (child_l != -1) {
        shmem_long_get(& lchild_ready,& pSync[0],1, child_l);
        while(lchild_ready != 0)
          shmem_long_get(& lchild_ready,& pSync[0],1, child_l);

        shmem_int_put(target_ptr, source_ptr, nlong, child_l);
        shmem_fence();  
        shmem_long_put(& pSync[0],& is_ready, 1, child_l);  
        no_children = 1;
      }
      if(child_r != -1) {
        shmem_long_get(& rchild_ready,& pSync[0],1, child_r);
        while(rchild_ready != 0)
          shmem_long_get(& rchild_ready,& pSync[0],1, child_r);

        shmem_int_put(target_ptr, source_ptr, nlong, child_r);  
        shmem_fence();  
        shmem_long_put(& pSync[0],& is_ready, 1, child_r);
        no_children = 2;    
      }
      pSync[0] =  _SHMEM_SYNC_VALUE;

      if(no_children == 0){
        pSync[1] =  _SHMEM_SYNC_VALUE;
        /*TO
         * DO:
         * Is
         * check
         * for
         * parents
         * pSync
         * required?*/
        shmem_long_inc(& pSync[1], parent);
      }
      else{
        shmem_long_wait_until(& pSync[1], SHMEM_CMP_EQ, (long)no_children);
        pSync[1] =  _SHMEM_SYNC_VALUE;
        /*printf("PE
         * %d
         * incrementing
         * child
         * count
         * on
         * PE
         * %d\n",my_pe,parent);*/
        shmem_long_inc(& pSync[1], parent);
      }

    }
    __shmem_trace(SHMEM_LOG_BROADCAST,
        "at the end of bcast32"
        );
    /* shmem_barrier(PE_start,
     * logPE_stride,
     * PE_size,
     * pSync);*/
  }


} 
  void
__shmem_broadcast64_tree(void *target, const void *source, size_t nlong,
    int PE_root, int PE_start,
    int logPE_stride, int PE_size,
    long *pSync)
{
  __shmem_broadcast32_tree(target, source, nlong * 2, PE_root, PE_start, logPE_stride, PE_size, pSync);

}

#include "module_info.h"
module_info_t module_info =
{
  __shmem_broadcast32_tree,
  __shmem_broadcast64_tree,
};

