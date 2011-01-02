/*------------------------------------------------------------------------
*
*    R6 is the output routine for Benchmark #6. 
*
*    Parameters:
*
*    Provided by the calling routine:
*	slen	=  Number of bits in the S stream
*	pattern	=  Bit pattern to be found in S stream
*	msize	=  Size of linear system to solve for each match
*	nloops	=  Number of problems (iterations) to do
*	matches =  Number of pattern matches found (last prob)
*	starts	=  Starting position for sets of equations (last prob)
*	indep	=  Flag for whether equations are indep or dep (last prob)
*	checks	=  For indep systems, number of eq's that don't check(last prob)
*	loopinfo=  Summary info from each problem (pass thru loop)
*	eqnw	=  Number of words to pack 1 equation
*	mype	=  my PE number
*	npes	=  Number of PEs
*	mpes	=  Number of PEs used for pattern matching
*	firstsol=  Solution for first indep system on each PE
*	solindx =  Index of first sol
*	cset	=  Total setup time (cpu)
*	wset	=  Total setup time (wall clock)
*	crun	=  Total run time (cpu)
*	wrun	=  Total run time (wall clock)
*	cchk	=  Total time to check solutions (cpu)
*	wchk	=  Total time to check solutions (wall clock)
*
*-----------------------------------------------------------------------*/

#include "bench6.h"
#include <stdio.h>

static char cvs_info[] = "BMARKGRP $Date: 2005/09/20 14:55:38 $ $Revision: 1.3 $ $RCSfile: r6.c,v $ $Name:  $";


/* arrays defined here so they are "symmetric" for shmem operations */
int allindep  [ MAXMATCH0 ];
int allchecks [ MAXMATCH0 ];
int64 allsolindx [ MAXPES ];

int pWrk[MAXMATCH0/2 + 1];	/* big enough for indep, checks */
/* int pWrk[_SHMEM_REDUCE_MIN_WRKDATA_SIZE]; THIS IS TOO SMALL */
long pSync[_SHMEM_REDUCE_SYNC_SIZE];


void
r6 ( int slen, char * pattern, int msize, int nloops, int matches,
	int * starts,	/* starts[matches] */		/* SYMMETRIC */
	int * indep,	/* indep [matches] */		/* SYMMETRIC */
	int * checks,	/* checks[matches] */		/* SYMMETRIC */
	int loopinfo[MAXLOOPS][5],
	int eqnw, int mype, int npes, int mpes,
	uint64 * firstsol,				/* SYMMETRIC */
	int solindx,					/* SYMMETRIC */
	double cset, double wset, double crun, double wrun,
	double cchk, double wchk )
{
   int j;
   int errck1 = 0, errck2 = 0;
   char * date();


/*----------------------------------------------------------------------------*/
   if ( mype == 0 )
   {

      printf("\n\n\nBenchmark #6 - Equation Solving - Shmem Version\n");

      printf("Date: %s\n", date());
 
      printf("Number of bits in S stream (SLEN) = %d\n", slen);
      printf("Pattern to search for             = %s\n", pattern);
      printf("Number of equations (matrix size) = %5d\n\n", msize);
      printf("Number of problems executed       = %5d\n", nloops);
      printf("Number of processors              = %5d\n",   npes);
      printf("Number of PEs for patt. matching  = %5d\n\n", mpes);

      printf("Total set up time\n");
      printf("   CPU    = %12.4f seconds\n", cset);
      printf("   WALL   = %12.4f seconds\n", wset);

      printf("Total time for equation solving:\n");
      printf("   CPU    = %12.4f seconds\n", crun);
      printf("   WALL   = %12.4f seconds\n", wrun);

      printf("Total time to verify solutions:\n");
      printf("   CPU    = %12.4f seconds\n", cchk);
      printf("   WALL   = %12.4f seconds\n", wchk);

      /*------------------------------------------------------------------------
      *  Output for passes 1 through NLOOPS
      *
      *  If slen != 1 000 000 or 10 000 000, no check for correctness of counts
      *-----------------------------------------------------------------------*/

      printf("\n\nResults for each loop\n");

      if ( loopinfo[0][3] == -1 )
      {
         printf("\tcannot check counts for this value of SLEN, so ERRTYPE=-1\n");
         printf("\t#INCORRECT SOLS should all be 0\n\n");
      }
      else
         printf("(ERRTYPE and #INCORRECT SOLS should all be 0)\n\n");

      printf("LOOP  MATCHES  INDEP   DEP   TOOFAR  ERRTYPE  #INCORRECT SOLS\n");

/*------------------------------------------------------------------------------
LOOP  MATCHES  INDEP   DEP   TOOFAR  ERRTYPE  #INCORRECT SOLS
..1.....269 .....87 ...175......7 ......0........0
12345678901234567890123456789012345678901234567890
 I3      I8X     I7X    I6     I7X     I7       I9
	   ^       ^		 ^	<<< 3 counts that may be flagged with *
------------------------------------------------------------------------------*/

#if 0
/*--------------------------------------------------------------------------
*  code to print out new tables of results for C6
*---------------------------------------------------------------------------*/
{
      int i, j, k;

      printf("\n\n\nNew Check Values for slen = %d\n\n", slen);

      for ( k=0; k<=2; ++k )
      {
         for ( i=0; i<10; ++i )
         {
            for ( j=0; j<10; ++j )
            {
               printf( "%3d, ", loopinfo[j+10*i][k] );
            }
            printf("\n");
         }
         printf("\n");
      }
      exit(0);
}
/*----------------------------------------------------------------------------*/
#endif

      /* print for each loop; 4 ways since 3 counts may be flagged with '*'  */

      for ( j = 0; j < nloops; ++j )
      {
         if ( loopinfo[j][3] <= 0 )  /* 0:okay  -1:no check for this slen */
             printf( "%3d%8d %7d %6d%7d %7d%9d\n",
		j+1, loopinfo[j][0], loopinfo[j][1], loopinfo[j][2],
		(loopinfo[j][0] - loopinfo[j][1] - loopinfo[j][2]),
		loopinfo[j][3], loopinfo[j][4] );

         if ( loopinfo[j][3] == 1 )	/* nmatches wrong */
             printf( "%3d%8d*%7d %6d%7d %7d%9d\n",
		j+1, loopinfo[j][0], loopinfo[j][1], loopinfo[j][2],
		(loopinfo[j][0] - loopinfo[j][1] - loopinfo[j][2]),
		loopinfo[j][3], loopinfo[j][4] );

         if ( loopinfo[j][3] == 2 )	/* ngmatches, toofar wrong */
             printf( "%3d%8d %7d %6d%7d*%7d%9d\n",
		j+1, loopinfo[j][0], loopinfo[j][1], loopinfo[j][2],
		(loopinfo[j][0] - loopinfo[j][1] - loopinfo[j][2]),
		loopinfo[j][3], loopinfo[j][4] );

         if ( loopinfo[j][3] == 3 )	/* nindep wrong */
             printf( "%3d%8d %7d*%6d%7d %7d%9d\n",
		j+1, loopinfo[j][0], loopinfo[j][1], loopinfo[j][2],
		(loopinfo[j][0] - loopinfo[j][1] - loopinfo[j][2]),
		loopinfo[j][3], loopinfo[j][4] );

         errck1 += loopinfo[j][3];
         errck2 += loopinfo[j][4];
      }

      if ( (errck1+errck2) > 0 )
         printf ( "\nTHERE ARE ERRORS IN THE RESULTS\n" );


   }  /* end PE0 print first part of output */

#ifdef PRINTALL		/***** start big conditional compile *****/

{  /* this is here so text editor can find end of conditional code */

   int i, penum;
   int64 solindx64 = solindx;

   shmem_barrier_all();

/*------------------------------------------------------------------------------
*  For last problem/loop, print out all occurrences of:
*	the starting position of the equations,
*	the flag indicating if the equations are dependent or independent
*	  (or "too far", not enough bits left in stream for set of eq's),
*	and the unique solution (if it exists) for the first set of
*	  independent equations.
*
*  ***** This only prints if the makefile has -DPRINTALL. *****
*-------------------------------------------------------------------------------
*  First collect all of indep(i) and checks(i) to PE0 (already has starts(i))
*  Also gather all the solindx's to allsolindx on all PEs
*-----------------------------------------------------------------------------*/
   if ( npes > 1 )
   {
      for ( i=0; i<_SHMEM_REDUCE_SYNC_SIZE; ++i )
         pSync[i] = _SHMEM_SYNC_VALUE;
      shmem_barrier_all();

      shmem_int_sum_to_all ( allindep, indep, matches, 0,0,npes, pWrk, pSync );

      shmem_int_sum_to_all ( allchecks, checks, matches, 0,0,npes, pWrk,pSync );

      shmem_fcollect64 ( allsolindx, &solindx64, 1, 0,0,npes, pSync );
   }
   else
   {
      for ( i = 0; i < matches; ++i )
      {
         allindep[i]  = indep[i];
         allchecks[i] = checks[i];
      }
      allsolindx[0] = solindx;
   }

/*------------------------------------------------------------------------------
*  Back to PE0 do all the work
*
*  Print info for each match in last loop
*-----------------------------------------------------------------------------*/

   if ( mype == 0 )
   {
      printf("\n\n          OUTPUT FOR LAST PROBLEM\n");

/*		1234567890123456789012345678901234567890123	*/
/*		      287430		  1		  0	*/
      printf("\nStarting position      	Flag:           Check:   \n");
      printf(  "                       	1=indep	        if indep,\n");
      printf(  "                        0=dep           # of eqs \n");
      printf(  "                        -1=too far     	that dont\n");
      printf(  "                                         check   \n");

      for ( i = 0; i < loopinfo[nloops-1][0]; ++i )
      {
         if ( allindep[i] == 1 )
         {
            printf(    "%12d             %3d             %3d\n",
		   starts[i],              1,      allchecks[i] );
         }
         else
         {
            printf(    "%12d             %3d\n",
		   starts[i],    allindep[i] );
         }
      }

/*------------------------------------------------------------------------------
*  Print first solution (if any) of last loop.
*
*  If it's not on PE0 (which happens only if this is a small test case?), 
*  PE0 figures which PE has it, fetches it from there, and prints it
*
*  HOW TO TEST THIS:  npes=3, slen=550000:  gmatches=3, solution on PE1
*-----------------------------------------------------------------------------*/

      for ( penum = 0; penum < npes; penum++ )
      {
         if ( allsolindx[penum] >= 0 )
         {
            /* the first solution is on PE penum -- if necessary PE0 pulls it */
            if ( penum > 0 )
               shmem_get64 ( firstsol, firstsol, eqnw, penum );

            printf("\n\nSolution for first set of indep equations in last problem (bits reversed):\n");

            for ( i = 0; i < eqnw; ++i )
            {
//#ifdef CRAY
//               printf( "%064B\n", firstsol[i] );
//#else
               uint64 word = firstsol[i];
               for ( j = 63; j >= 0; --j )
                  printf( "%d", (int) ( (word >> j) & (uint64)1 ) );
               printf("\n");
//#endif
            }
            solindx = 1;  /* flag for below */
            break;  /* stop after the first one */
         }  /* end first solution */

      }  /* end for-penum-loop */

      if ( solindx == -1 )
      {
         printf("\n\nThere is no indep system of equations in last problem\n");
      }

   }  /* end if mype == 0 */

}  /***** end of conditional compile for PRINTALL *****/

#endif

   if ( mype == 0 )
      printf("\n\n");

   shmem_barrier_all();
}  /* R6 */
