/*
 *
 * Copyright (c) 2011, 2012, University of Houston System and Oak Ridge National
 * Laboratory.
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * o Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * 
 * o Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 
 * o Neither the name of the University of Houston System, Oak Ridge
 *   National Laboratory nor the names of its contributors may be used to
 *   endorse or promote products derived from this software without specific
 *   prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * This program shows how to use shmem_fcollect to simulate MPI_Alltoall
 * Each processor send/rec a different  random number to/from other processors.  
 * Adopted/ported from source url: http://geco.mines.edu/workshop/class2/examples/mpi/c_ex07.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <shmem.h>

/* globals */
int numnodes,myid,mpi_err;
#define mpi_root 0
/* end module  */

void init_it(int  *argc, char ***argv);
void seed_random(int  id);
void random_number(float *z);

void init_it(int  *argc, char ***argv) {
    //mpi_err = MPI_Init(argc,argv);
    //mpi_err = MPI_Comm_size( MPI_COMM_WORLD, &numnodes );
    //mpi_err = MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    start_pes(0);
    numnodes = _num_pes();
    myid = _my_pe();
}


int main(int argc,char *argv[]){
	int *sray,*rray;
	int *sdisp,*scounts,*rdisp,*rcounts,*rcounts_full;
	int ssize,rsize,i,k,j;
	float z;

	init_it(&argc,&argv);
	scounts=(int*)shmalloc(sizeof(int)*numnodes);
	rcounts=(int*)shmalloc(sizeof(int)*numnodes);
	rcounts_full=(int*)shmalloc(sizeof(int)*numnodes*numnodes);
	sdisp=(int*)shmalloc(sizeof(int)*numnodes);
	rdisp=(int*)shmalloc(sizeof(int)*numnodes);
/*
! seed the random number generator with a
! different number on each processor
*/
	seed_random(myid);
/* find out how much data to send */
	for(i=0;i<numnodes;i++){
		random_number(&z);
		scounts[i]=(int)(5.0*z)+1;
	}
	printf("myid= %d scounts=%d %d %d %d\n",myid,scounts[0],scounts[1],scounts[2],scounts[3]);
	printf("\n");
/* tell the other processors how much data is coming */
	//mpi_err = MPI_Alltoall(scounts,1,MPI_INT, rcounts,1,MPI_INT, MPI_COMM_WORLD);
        static long psync[_SHMEM_COLLECT_SYNC_SIZE];
	for(i=0;i<_SHMEM_COLLECT_SYNC_SIZE;i++)
		psync[i] = _SHMEM_SYNC_VALUE;
        shmem_barrier_all();
        int other,j1;
        shmem_fcollect32(rcounts_full,scounts,4,0,0,numnodes,psync);
        for(i=0;i<numnodes;i++) {
		rcounts[i]=rcounts_full[i*numnodes+myid];
	}
        printf("-----myid= %d rcounts=",myid);
        for(i=0;i<numnodes;i++)
                printf("%d ",rcounts[i]);
        printf("\n");


/*	write(*,*)"myid= ",myid," rcounts= ",rcounts */
/* calculate displacements and the size of the arrays */
	sdisp[0]=0;
	for(i=1;i<numnodes;i++){
		sdisp[i]=scounts[i-1]+sdisp[i-1];
	}
	rdisp[0]=0;
	for(i=1;i<numnodes;i++){
		rdisp[i]=rcounts[i-1]+rdisp[i-1];
	}
	ssize=0;
	rsize=0;
	for(i=0;i<numnodes;i++){
		ssize=ssize+scounts[i];
		rsize=rsize+rcounts[i];
	}
	
/* allocate send and rec arrays */
	sray=(int*)shmalloc(sizeof(int)*20);
	rray=(int*)shmalloc(sizeof(int)*20);
	for(i=0;i<ssize;i++){
		sray[i]=myid;
	}
/* send/rec different amounts of data to/from each processor */
	//mpi_err = MPI_Alltoallv(sray,scounts,sdisp,MPI_INT, rray,rcounts,rdisp,MPI_INT, MPI_COMM_WORLD);
        shmem_barrier_all();
        for(j1=0;j1<numnodes;j1++){
                int k1=sdisp[j1];
                static int k2;
                shmem_int_get(&k2,&rdisp[myid],1,j1);
                shmem_int_put(rray+k2,sray+k1,scounts[j1],j1);
        }
        shmem_barrier_all();

        // not possible, coz even though the rcounts[myid] will be different on each PE, the elements collected
        // by PE0 from other PE's will be constant. 
        //shmem_collect32(rray_full,sray,rcounts[myid],0,0,numnodes,psync);
	                
	printf("myid= %d rray=",myid);
	for(i=0;i<rsize;i++)
		printf("%d ",rray[i]);
	printf("\n");
    //mpi_err = MPI_Finalize();
}
/*
  0:myid= 0 scounts=1 7 4
  0:myid= 0 scounts=0 1 1 1 1 1 1 2
  1:myid= 1 scounts=6 2 4
  1:myid= 1 scounts=0 0 0 0 0 0 0 1 1 2 2 2 2 2 2 2
  2:myid= 2 scounts=1 7 4
  2:myid= 2 scounts=0 0 0 0 1 1 1 1 2 2 2 2
*/

void seed_random(int  id){
	srand((unsigned int)id);
}
void random_number(float *z){
	int i;
	i=rand();
	*z=(float)i/RAND_MAX;
}
