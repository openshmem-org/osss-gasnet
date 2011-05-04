#ifndef _CLOCK_H
#define _CLOCK_H 1

/*
 * start the clock subsystem
 *
 */
extern void __shmem_elapsed_clock_init(void);

/*
 * read the clock to see how much time has elapsed
 *
 */
extern double __shmem_get_elapsed_clock(void);

#endif /* _CLOCK_H */
