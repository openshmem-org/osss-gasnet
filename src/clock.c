#include <stdio.h>
#include <sys/time.h>

static double epoch;

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

void
__shmem_elapsed_clock_init(void)
{
  epoch = read_clock();
}

double
__shmem_get_elapsed_clock(void)
{
  double now = read_clock();

  return now - epoch;
}
