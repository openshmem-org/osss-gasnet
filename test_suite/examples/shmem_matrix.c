/* (c) 2011 University of Houston System.  All rights reserved. 
 * Program to calculate the product of 2 matrices A and B based on block distribution.
 * Adopted from the mpi implementation of matrix muliplication based on 1D block-column distribution.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpp/shmem.h>
#include <sys/time.h>
#include <unistd.h>

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


/* set the number of rows and column here */
#define ROWS 8 
#define COLUMNS 8 

// routine to print the partial array
void print_array(double **array,int blocksize)
{
 int i,j;
 for(i=0; i<ROWS;i++){
      for(j=0; j<blocksize; j++){
         printf("%f  ",array[i][j]);
      }//end for loop j
      printf("\n");
 }//end for loop i
      printf("\n");
      printf("\n");
}

// needed for reduction operation
long pSync[_SHMEM_BCAST_SYNC_SIZE]; 
double pWrk[_SHMEM_REDUCE_SYNC_SIZE];

// global shmem_accesible
double maxtime; 
double         t,tv[2];

int main ( int argc, char **argv )
{
  int i, j, k;
  int blocksize;
  int rank, size,nextpe;
  int p, np; // round and number of process
  double **a_local, **b_local;
  double **c_local;
  int B_matrix_displacement;

  for (i = 0; i < _SHMEM_BCAST_SYNC_SIZE; i += 1) {
    pSync[i] = _SHMEM_SYNC_VALUE;
  }

  tv[0] = gettime();

  start_pes(0);
  rank = _my_pe();
  size = _num_pes();

  np = size; // number of processes
  blocksize = COLUMNS/np; // block size
  B_matrix_displacement = rank * blocksize ;

  shmem_barrier_all();
  a_local = (double **)shmalloc(ROWS*sizeof(double *));
  b_local = (double **)shmalloc(ROWS*sizeof(double *));
  c_local = (double **)shmalloc(ROWS*sizeof(double *));
  for(i=0; i<ROWS; i++) {
        a_local[i] = (double *)shmalloc(blocksize*sizeof(double));
        b_local[i] = (double *)shmalloc(blocksize*sizeof(double));
        c_local[i] = (double *)shmalloc(blocksize*sizeof(double));
        for(j=0; j<blocksize ; j++) {
                a_local[i][j]= i+1*j+1*rank+1; // random values
                b_local[i][j] = i+2*j+2*rank+1; // random values
                c_local[i][j]=0.0 ;
        }
  }


 shmem_barrier_all();
 #ifdef DEBUG // print the input arrays from root process
 if(rank==0)  {
	printf("matrix a from %d\n",rank);
  	print_array(a_local,blocksize);
	printf("matrix b from %d\n",rank);
  	print_array(b_local,blocksize);
 }
 #endif
 shmem_barrier_all();

 for(i=0; i<ROWS; i++) {
        for(p=1; p<=np; p++) {
                for(k=0; k<blocksize; k++) {
                        for(j=0; j<blocksize; j++) {
                          	c_local[i][j] = c_local[i][j] + a_local[i][k] * b_local[k+B_matrix_displacement][j];
                        }
 		}
 	shmem_barrier_all();
 	if(rank == np-1) 
      		shmem_double_put(&a_local[i][0],&a_local[i][0],blocksize,0);
 	else 
       		shmem_double_put(&a_local[i][0],&a_local[i][0],blocksize,rank+1);
 	shmem_barrier_all();
 	if(B_matrix_displacement == 0)
        	B_matrix_displacement = (np-1) * blocksize;
 	else
        	B_matrix_displacement = B_matrix_displacement-blocksize;
 	}
 }

 shmem_barrier_all();
 tv[1] = gettime();
 t = dt (&tv[1], &tv[0]);
#if DEBUG
 printf("Process %d: %4.2f Sec \n",rank,t/1000000.0);
#endif

 shmem_double_max_to_all(&maxtime, &t, 1, 0, 0, size, pWrk, pSync);

 #if DEBUG // print the resultant array from root process
 if(rank==0) {
	printf("matrix c from %d\n",rank);
  	print_array(c_local,blocksize);
 }
 #endif

 if(rank==0)
    printf("Maximum time =%4.2f seconds\n",maxtime/1000000.0);

 return (0);
}
