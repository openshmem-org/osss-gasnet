/* (c) 2011 University of Houston.  All rights reserved. */


#ifndef _CLOCK_H
#define _CLOCK_H 1

/*
 * start/stop the clock subsystem
 *
 */
extern void __shmem_elapsed_clock_init(void);
extern void __shmem_elapsed_clock_finalize(void);

/*
 * read the clock to see how much time has elapsed
 *
 */
extern double __shmem_elapsed_clock_get(void);

#endif /* _CLOCK_H */
