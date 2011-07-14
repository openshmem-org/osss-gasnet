/* (c) 2011 University of Houston System.  All rights reserved. */


#include <stdio.h>
#include <sys/time.h>

/*
 * record start of program run
 */
static double epoch;

/*
 * return number of (fractional) seconds
 * since program started
 */
static
double
read_clock(void)
{
  struct timeval tv;
  double t;

  gettimeofday(&tv, (struct timezone *) NULL);

  t = (double) tv.tv_sec;
  t += (double) tv.tv_usec / 1000000.0;

  return t;
}

/*
 * start the clock running
 */
void
__shmem_elapsed_clock_init(void)
{
  epoch = read_clock();
}

/*
 * stop the clock
 */
void
__shmem_elapsed_clock_finalize(void)
{
  return;
}

/*
 * read the current run time
 */
double
__shmem_elapsed_clock_get(void)
{
  double now = read_clock();

  return now - epoch;
}
