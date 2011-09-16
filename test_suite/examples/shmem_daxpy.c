/* (c) 2011 University of Houston System.  All rights reserved. 
 * A simple DAXPY like sample kernel with computation and communication.
 * Adopted/ported from source url: http://parallel-for.sourceforge.net/shmem-proc-cpu-scalar.html 
 */
#include <mpp/shmem.h>
#include <stdio.h>

// global shmem_accesible
double maxtime;
double         t,tv[2];

double gettime()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return (tv.tv_sec * 1000000 + tv.tv_usec);
}

double dt (double *tv1, double *tv2)
{
  return (*tv1 - *tv2);
}

int main(int argc, char *argv[]) {
  int n = 16;
  int i;
  static float pWork[_SHMEM_REDUCE_SYNC_SIZE];
  static long pSync[_SHMEM_BCAST_SYNC_SIZE];
  static double dpWrk[_SHMEM_REDUCE_SYNC_SIZE];
  static long sync[_SHMEM_REDUCE_MIN_WRKDATA_SIZE];
  static float el, es;
  int my_pe,num_pes;

  for (i = 0; i < SHMEM_BCAST_SYNC_SIZE; i += 1) {
    pSync[i]  =_SHMEM_SYNC_VALUE;
  }

  tv[0] = gettime();

  start_pes(0);
  num_pes = _num_pes();
  my_pe = _my_pe();
  int nn = (n-1) / num_pes; 
  int n_local0 = 1 + my_pe * nn;
  int n_local1 = 1 + (my_pe+1) * nn; 
  // allocate only local part + ghost zone of the arrays x,y
  float *x, *y;
  x = (float*)shmalloc((n_local1 - n_local0 + 2)*sizeof(float)); 
  y = (float*)shmalloc((n_local1 - n_local0 + 2)*sizeof(float));
  x -= (n_local0 - 1); 
  y -= (n_local0 - 1);
  shmem_barrier_all();

  //... // fill x, y
  for (i=n_local0; i<n_local1; ++i) {
  	x[i]=5;
  	y[i]=5;
  }

  // fill ghost zone
  if (my_pe > 0)
   shmem_float_put(&y[n_local1], &y[n_local0], 1, my_pe-1);
  if (my_pe < num_pes-1)
   shmem_float_put(&y[n_local0-1], &y[n_local1-1], 1, my_pe+1);
  shmem_barrier_all();

  // do computation
  float e = 0;
  for (i=n_local0; i<n_local1; ++i) {
   x[i] += ( y[i+1] + y[i-1] )*.5;
   e += y[i] * y[i];
  }

  el = e;
  shmem_float_sum_to_all(&es, &el, 1, 0, 0, num_pes, pWork, pSync);

  e = es;

  //... // output x, e
  for (i=n_local0; i<n_local1; ++i) {
  	printf("x%d[%d]=%f \n",my_pe,i,x[i]);
  }
  printf("\n");

  x += (n_local0 - 1); // x=x,x=x+3
  y += (n_local0 - 1); // y=y,y=y+3
  shfree(x);
  shfree(y);

  tv[1] = gettime();
  t = dt (&tv[1], &tv[0]);
  shmem_double_max_to_all(&maxtime, &t, 1, 0, 0, num_pes, dpWrk, pSync);

  if(my_pe==1)
    printf("Maximum time =%f\n",maxtime/1000000.0);

  return 0;
} 
