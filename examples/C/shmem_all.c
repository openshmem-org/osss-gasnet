/*
 *
 * Copyright (c) 2011 - 2014
 *   University of Houston System and Oak Ridge National Laboratory.
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
 * This program shows how to use shmem_put to simulate MPI_Alltoall.  
 * Each processor send/rec a different  random number to/from other processors.  
 * Adopted/ported from source url: http://geco.mines.edu/workshop/class2/examples/mpi/c_ex07.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpp/shmem.h>


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
    myid = shmem_my_pe();
}

int main(int argc,char *argv[]){
	int *sray,*rray;
	int *sdisp,*scounts,*rdisp,*rcounts;
	int ssize,rsize,i,k,j;
	float z;

	init_it(&argc,&argv);
	scounts=(int*)shmalloc(sizeof(int)*numnodes);
	rcounts=(int*)shmalloc(sizeof(int)*numnodes);
	sdisp=(int*)shmalloc(sizeof(int)*numnodes);
	rdisp=(int*)shmalloc(sizeof(int)*numnodes);
	/*
	! seed the random number generator with a
	! different number on each processor
	*/
	seed_random(myid);
	/* find  data to send */
	for(i=0;i<numnodes;i++){
		random_number(&z);
		scounts[i]=(int)(10.0*z)+1;
	}
	printf("-------myid= %d scounts=",myid);
	for(i=0;i<numnodes;i++)
		printf("%d ",scounts[i]);
	printf("\n");
	/* send the data */
	//mpi_err = MPI_Alltoall(scounts,1,MPI_INT, rcounts,1,MPI_INT, MPI_COMM_WORLD);
        shmem_barrier_all();
        int other,j1;
        for(j1=0;j1<numnodes;j1++){
                shmem_int_put(&rcounts[myid],&scounts[j1],1,j1);
        }
        shmem_barrier_all();
	printf("myid= %d rcounts=",myid);
	for(i=0;i<numnodes;i++)
		printf("%d ",rcounts[i]);
	printf("\n");
}

void seed_random(int  id){
	srand((unsigned int)id);
}
void random_number(float *z){
	int i;
	i=rand();
	*z=(float)i/RAND_MAX;
}
