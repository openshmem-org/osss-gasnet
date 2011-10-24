
#include "mpi.h"
#include <sys/time.h>
#include <unistd.h>

double shmem_time()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return (tv.tv_sec);
}

double start[64], elapsed[64];

/*****************************************************************/
/******            T  I  M  E  R  _  C  L  E  A  R          ******/
/*****************************************************************/
void timer_clear( int n )
{
    elapsed[n] = 0.0;
}


/*****************************************************************/
/******            T  I  M  E  R  _  S  T  A  R  T          ******/
/*****************************************************************/
void timer_start( int n )
{
    start[n] = shmem_time();
}


/*****************************************************************/
/******            T  I  M  E  R  _  S  T  O  P             ******/
/*****************************************************************/
void timer_stop( int n )
{
    double t, now;

    now = shmem_time();
    t = now - start[n];
    elapsed[n] += t;

}


/*****************************************************************/
/******            T  I  M  E  R  _  R  E  A  D             ******/
/*****************************************************************/
double timer_read( int n )
{
    return( elapsed[n] );
}

