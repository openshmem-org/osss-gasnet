/*
 * SGI/SHMEM version of the C "pi" program that is part of the MPICH
 * distribution
 *
 * MPI version is:
 *  (C) 2001 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory (of MPICH distribution).
 */

#include <mpp/shmem.h>
#include <stdlib.h>

#include <stdio.h>
#include <math.h>

static const double PI25DT = 3.141592653589793238462643;

double f(double a)
{
  return (4.0 / (1.0 + a*a));
}

int n;

long pSync[SHMEM_BCAST_SYNC_SIZE];

double mypi, pi;
double pWrk[SHMEM_REDUCE_SYNC_SIZE];

int main(int argc,char *argv[])
{
  int    myid, numprocs, i;
  double h, sum, x;
  double startwtime = 0.0, endwtime;

  start_pes(0);
  numprocs = _num_pes();
  myid = _my_pe();

  if (argc > 1)
    n = atoi(argv[1]);		/* # rectangles on command line */
  else
    n = 10000;			/* default # of rectangles */

  if (myid == 0)
    time(&startwtime);

  shmem_sync_init(pSync);

  shmem_barrier_all();

  shmem_broadcast32(&n, &n, 1, 0, 0, 0, numprocs, pSync);

  h   = 1.0 / (double) n;
  sum = 0.0;
  /* A slightly better approach starts from large i and works back */
  for (i = myid + 1; i <= n; i += numprocs)
    {
      x = h * ((double)i - 0.5);
      sum += f(x);
    }
  mypi = h * sum;

  shmem_barrier_all();

  shmem_double_sum_to_all(&pi, &mypi, 1, 0, 0, numprocs, pWrk, pSync);

  shmem_barrier_all();

  if (myid == 0) {
    time(&endwtime);
    printf("pi is approximately %.16f, Error is %.16f\n",
	   pi, fabs(pi - PI25DT));
    printf("wall clock time = %f\n", endwtime-startwtime);	       
    fflush(stdout);
  }

  return 0;
}
