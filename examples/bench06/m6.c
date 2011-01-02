/*
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c  Benchmark #6  -- Equation Solving
c
c
c  Let {S(i)} be a stream of binary digits of length SLEN.  Search for all
c  occurrences of the pattern P where certain bits in the pattern may
c  be either 1 or 0 ("don't care"), and let j be the position in the
c  stream immediately after an occurrence of P.
c
c     Example:
c       Assume P = 000???100??110?10?1111 where "?" may be either 1 or 0.
c     Then in the stream:
c
c                  011010001110101000001011001111001001111101...
c     we have                       000???100??110?10?1111
c
c                                   <--------- P -------->j
c
c  Starting at position j, use the next 700 x 701 bits of the stream
c  to form 700 equations in 700 unknowns x(i) over GF(2) as follows:
c
c  S(j       )x(1) + S(j +    1)x(2) +...+ S(j +  699)x(700)= S(j +  700)   (1)
c  S(j +  701)x(1) + S(j +  702)x(2) +...+ S(j + 1400)x(700)= S(j + 1401)   (2)
c  S(j + 1402)x(1) + S(j + 1403)x(2) +...+ S(j + 2101)x(700)= S(j + 2102)   (3)
c          .                 .                     .                  .
c          .                 .                     .                  .
c          .                 .                     .                  .
c  S(j+489999)x(1) + S(j+490000)x(2) +...+ S(j+490698)x(700)= S(j+490699) (700)
c
c  For each occurrence of the pattern P in {S(i)}, form the above equations
c  (as long as there are enough bits remaining in the stream) and determine
c  whether the equations are dependent or independent.
c  If the equations are independent, then find the unique solution.
c  Gaussian elimination over GF(2) with block reduction is the suggested method.
c
c  Parameters:  NLOOPS = 100   SLEN = 10**7
c
c  Output the following counts for each loop:
c    matches	number of pattern matches found in the stream
c    indep	number of independent systems of equations
c    dep	number of dependent systems of equations
c    toofar	number of pattern matches too far in the stream to form eq's
c    error type:	0	no errors in these counts
c			1	matches incorrect
c			2	matches correct, but toofar incorrect
c			3	matches & toofar correct, but indep incorrect
c    number of indep systems whose solutions do not check
c
c  IF the makefile has -DPRINTALL, THEN print two more sections of results:
c
c  Output for all matches in the last loop, the list (j flag {check}) where:
c    j = starting position of the equations (stream position after pattern)
c    flag  = { 1, if the equations are independent
c	       0, if the equations are dependent
c	      -1, if the match is toofar }
c    check = if independent, number of equations in the system for which
c	     the solution found does not check
c
c  Finally, output the solution of the first independent system in the last loop
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c  Main Program for Benchmark #6 - Shmem Version
c
c  Call time parameters:
c
c     NLOOPS =  Number of problems, as described above, the program executes.
c		Default = 100
c		Maximum = 400  (But Max is 100 for SLEN = 10**6)
c
c		NOTE:  In order to remove the complexity of using IO
c		to secondary storage, this benchmark is repeated NLOOPS
c		times (on different data each time) and the time is
c		accumulated.  This will NOT be considered a legitimate
c		loop for parallelization, since it is done ONLY to
c		reduce the complexity of the problem.
c
c     MPES   =  Number of PEs to use for pattern matching.
c		Default = NPES, the full number of PEs to do the benchmark.
c
c     SLEN   =  Length of the bit stream
c		Default (and Maximum) = 10000000
c		Other checkable value = 1000000
c
c		NOTE:  Check routine verifies answers only for 10**6 and 10**7
c
c     optional parameters for smaller test cases:
c
c     MSIZE  =	Size of linear system to solve for each pattern match
c		Default (and Maximum) = 700
c
c     PATTERN=  The bit pattern to match, with "x" representing a don't-care bit
c		Default = "000xxx100xx110x10x1111"
c		Maximum length = 31
c		Be sure to enclose it in (single  or double) quotes
c			0x1100101 would be a hex number otherwise.
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
*/

#include "bench6.h"
#include <string.h>

#include <stdlib.h> /* exit() */

#include <assert.h>

static char cvs_info[] = "BMARKGRP $Date: 2005/01/10 21:17:26 $ $Revision: 1.2 $ $RCSfile: m6.c,v $ $Name:  $";


#include <mpp/shmem.h>
#if 0
int _num_pes(void);
int _my_pe(void);
#endif

extern void c6(), p6(), r6(), s6();
extern void initpc();

/*-------------------------------------------------------------------------
*  arrays defined here are not on stack so remotely accessible by shmem ops
*--------------------------------------------------------------------------*/
/* Array that winds up with pattern-match start points */
/* in P6, shmem_get64 pulls them up the tree of PEs */
   uint64	startbits [ SNWMAX ];

/* Starting position of each set of equations  */
/* in P6, PE0 broadcasts them to all PEs */
   int		starts [ MAXMATCH0 ];

/* Flag indicating whether or not each set of equations is independent */
/* in R6, shmem_int_sum_to_all to combine the partially filled segments */
   int		indep [ MAXMATCH0 ];

/* Check for solution to each indep set of equations */
/* in R6, shmem_int_sum_to_all to combine the partially filled segments */
   int		checks[ MAXMATCH0 ];

/* Number of independent sets of equations */
/* in C6, shmem_int_sum_to_all to total local counts */
   int		nindep;

/* Index of first indep set of eq's on a PE - among those sets it handles */
/* in R6, shmem_fcollect */
   int		solindx;

/* Solution to first indep set of eq's on a PE */
   uint64	firstsol [ EQNW0 ];


int
main(int argc, char**argv)
{
  /* malloc these instead to get them off the stack (they're too big) */

   uint64 *s;         /* The S-stream */
   uint64 *eqs;       /* sets of equations corresponding to pattern matches */
   uint64 *sols;      /* Solutions to independent sets of equations, packed */


/*-------------------------------------------------------------------------*/

/* The default pattern to be found (with x's in the don't care positions) */
   const char PAT[23]  = "000xxx100xx110x10x1111";

/* Output array holding info from each loop */
   int loopinfo[MAXLOOPS][5];

/* Variables for timing purposes */
   double cputime(), cset, crun, cchk, X0;
   double wall()   , wset, wrun, wchk, Y0;

   const int ws = WS;
   char pattern[32];
   char pchar;
   uint64 care_and_mask, care_xor_mask;
   int plen, snw;
   int slen, msize, eqnw, iseed, mype, npes, mpes;
   int nloops, loop;
   int i, matches, gmatches;
   int maxmatch;

/* This should come before any other executed code */
#ifdef CRAY
   shmem_init();
#endif /* CRAY */
#ifdef _SGI
   start_pes(0);
#endif /* _SICORTEX */

   npes = _num_pes();
   mype = _my_pe();

/*-------------------------------------------------------------------------*/

   s = (uint64 *) shmalloc( SNWMAX );
   assert(s != NULL);
   eqs = (uint64 *) shmalloc( MAXMATCH0 * MSIZE0 * EQNW0 );
   assert(eqs != NULL);
   sols = (uint64 *) shmalloc( MAXMATCH0 * EQNW0 );
   assert(sols != NULL);

/*-------------------------------------------------------------------------*/

   maxmatch = MAXMATCH0;
   iseed = ISEED0;

#ifdef SW_POPCNT
   initpc();
#endif


/*-------------------------------------------------------------------------
*  read in program arguments (first set defaults)
*--------------------------------------------------------------------------*/
   nloops = DEFLOOPS;
   mpes   = npes;
   slen   = SLENMAX;
   msize  = MSIZE0;
   strcpy ( pattern, PAT );

   if (argc > 1)	nloops = atoi(argv[1]);
   if (argc > 2)	mpes   = atoi(argv[2]);
   if (argc > 3)	slen   = atoi(argv[3]);
   if (argc > 4)	msize  = atoi(argv[4]);
   if (argc > 5)	strncpy(pattern, argv[5], 32);

/* check for 0=> default and for too large */

/* Defaults */
   if ( nloops <= 0 ) nloops = DEFLOOPS;
   if ( mpes   <= 0 ) mpes   = npes;
   if ( mpes > npes ) mpes   = npes;
   if ( slen   <= 0 ) slen   = SLENMAX;
   if ( msize  <= 0 ) msize  = MSIZE0;

/* Check for values too large */
   if (nloops > MAXLOOPS)
   {
      if (mype == 0)
         printf("\nInput value of NLOOPS too big, Max = %d\n", MAXLOOPS);
      exit(1); 
   }
   if (slen > SLENMAX)
  {
      if (mype == 0)
         printf("\nInput value of SLEN too big, Max = %d\n", SLENMAX);
      exit(1); 
   }
   if (msize > MSIZE0)
   {
      if (mype == 0)
         printf("\nInput value of MSIZE too big, Max = %d\n", MSIZE0);
      exit(1); 
   }

/* Number of words to hold bitstream S */
   snw = (slen+ws-1)/ws;
/* Number of words to hold one equation */
   eqnw = msize/ws + 1;


/*-------------------------------------------------------------------------
*  build binary masks from character array 'pattern'.
*  maximum value for plen is 31, even for 64-bit machines.
*  care_and_mask has 1's where the pattern specifies a 0 or a 1,
*	and 0's where the pattern has an x for don't-care.
*  care_xor_mask has 1's only where the pattern specifies a 1.
*  The bits are reversed so the first bit of the mask is least-significant,
*	to match the way the stream s is stored.
*--------------------------------------------------------------------------*/
   care_and_mask = 0;
   care_xor_mask = 0;
   plen = strlen(pattern);
   for ( i = 0; i < plen; ++i)
   {
      pchar = pattern[plen-1-i];
      if ( (pchar == '1') | (pchar == '0') )
      {
         care_and_mask |= (1 << i);
         if (pchar == '1')
            care_xor_mask |= (1 << i);
      }
   }

   cset = wset = 0.0;
   crun = wrun = 0.0;
   cchk = wchk = 0.0;
   wall();

/*-------------------------------------------------------------------------
*  Main loop - do everything nloops times
*--------------------------------------------------------------------------*/

   for ( loop = 0; loop < nloops; ++loop )
   {

/*-------------------------------------------------------------------------
*  S6: all PE's generate the random bits for the stream
*  and initialize startbits to all 1's except end
*-------------------------------------------------------------------------*/

      X0 = cputime();
      Y0 = wall();

#ifdef Debug
      printf("%2d  call S6\n", mype);
#endif

      /* loop+1 to agree with Fortran results */
      s6 ( s, startbits, slen, snw, plen, iseed, loop+1 );

      X0 = cputime() - X0;
      cset = cset + X0;
      Y0 = wall() - Y0;
      wset = wset + Y0;

/*-------------------------------------------------------------------------
*  P6 finds pattern matches, then solves resulting sets of equations.
*  The PE's divide the matches into chunks.
*--------------------------------------------------------------------------*/

/* zeroize these for last loop so at end can use shmem_int_sum_to_all */
/* to collect to PE0 for printing output */
      if ( loop == nloops - 1 )
         for ( i = 0; i < maxmatch; ++i )
         {
            indep[i]  = 0;
            checks[i] = 0;
         }

      shmem_barrier_all();

      X0 = cputime();
      Y0 = wall();

#ifdef Debug
      printf("%2d  call P6\n", mype);
#endif

      p6 ( s, startbits, eqs, sols, starts, indep,
		slen, snw, eqnw, msize, plen,
		care_and_mask, care_xor_mask,
#ifdef Timer
		loop+1, nloops,
#endif
		mype, npes, mpes,
		&matches, &gmatches, &nindep );

      shmem_barrier_all();

      X0 = cputime() - X0;
      crun = crun + X0;
      Y0 = wall() - Y0;
      wrun = wrun + Y0;

#ifdef Debug
      printf("%d  %d  %d  %d\n", mype, matches, gmatches, nindep);
#endif

/*-------------------------------------------------------------------------
*  C6 checks the solutions found to the indep sets of equations.
*  Each PE checks the chunks of equations/sols that it produced in P6.
*--------------------------------------------------------------------------*/

      X0 = cputime();
      Y0 = wall();

#ifdef Debug
      printf("%2d  call C6\n", mype);
#endif

      c6 ( loop, slen, eqs, sols, msize, eqnw, indep, checks, &nindep,
		loopinfo, mype, npes, matches, gmatches, firstsol, &solindx );

      X0 = cputime() - X0;
      cchk = cchk + X0;
      Y0 = wall() - Y0;
      wchk = wchk + Y0;


   }  /* end main loop */

/*-------------------------------------------------------------------------
*  R6 prints the results
*--------------------------------------------------------------------------*/

#ifdef Debug
      printf("%2d  call R6\n", mype);
#endif

   r6 ( slen, pattern, msize, nloops, matches,
		starts, indep, checks, loopinfo, eqnw,
		mype, npes, mpes, firstsol, solindx,
		cset, wset, crun, wrun, cchk, wchk );

   shmem_barrier_all();

  return 0;
}
