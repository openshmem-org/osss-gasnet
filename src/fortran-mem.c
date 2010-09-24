#include "shmem.h"

#include "fortran-common.h"

/*
 * symmetric memory operations
 */

/*
 * SYNOPSIS
 *        POINTER (addr, A(1))
 *        INTEGER (length, errcode, abort)
 *        CALL SHPALLOC(addr, length, errcode, abort)
 * 
 * DESCRIPTION
 *        SHPALLOC  allocates a block of memory from the program's symmetric heap
 *        that is greater than or equal  to  the  size  requested.   To  maintain
 *        symmetric  heap  consistency,  all PEs in an program must call SHPALLOC
 *        with the same value of length; if any  processing  elements  (PEs)  are
 *        missing, the program will hang.
 */
