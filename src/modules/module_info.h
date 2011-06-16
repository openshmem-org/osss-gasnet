/* (c) 2011 University of Houston.  All rights reserved. */


#ifndef _MODULE_INFO_H
#define _MODULE_INFO_H 1

/*
 * tell us which functions implement the 32 vs 64 bit routines
 */

typedef struct {
  void (*func_32)();
  void (*func_64)();
} module_info_t;

#endif /* _MODULE_INFO_H */
