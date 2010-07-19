#include "b10defs.h"
#include <time.h>

/*  This is an output routine for Benchmark #10.  
 *
 * This will need to be modified to contain the particular
 *  configuration information requested.
 *
 *   Parameters:
 *  Global external variables :
 *      N        =  Number of squares on one side of the chess board
 *      WIDTH    =  Degree of parallelism used
 *      SLVL     =  The number of levels of the tree to search before
 *                  dividing the resulting near-root nodes among processors 
 *      SOLS     =  An array containing the solutions found
 *
 *  Provided by calling routine:
 *      NSOLS    =  The number of solutions found
 *      crun     =  The amount of CPU time used to find these solutions
 *      wrun     =  The amount of wall clock time used to find these
 *                  solutions
 *      cverify  =  CPU time to verify the solutions
 *      wverify  =  Wall clock time to verify the solutions
 *      ERRNSOL  =  Holds 0 if correct number of solutions were found;
 *                  otherwise, holds correct number of solutions
 *      NERR     =  Number of invalid solutions found
 *      ERRORS   =  Array holding first 100 invalid solutions 
 */
 
 
static char cvs_info[] = "BMARKGRP $Date: 2005/01/10 21:17:39 $ $Revision: 1.2 $ $RCSfile: r10.c,v $ $Name:  $";
      
extern double cputime (), wall ();
extern int N, WIDTH, SLVL;
extern long int * SOLS;

void
r10 (int NSOLS, 
     double crun, double wrun, double cverify, double wverify,
     int ERRNSOL, int NERR, long int ERRORS[100][2])
{
   int    i, j, lim;

   printf ("\n\n\nBenchmark #10 -- N Queens\n\n");
   printf ("Date: %s at %s\n\n", __DATE__, __TIME__);
   printf ("Board size  = %d\n", N);
   printf ("Width (degree of parallelism) = %d\n", WIDTH);
   printf ("Tree levels = %d\n", SLVL);
   printf ("\nSolutions = %d\n", NSOLS);
   printf ("\nTime to run:\n");
   printf ("CPU time = %14.6f seconds\n", crun);
   printf ("Wall Clock = %14.6f seconds\n", wrun);
   printf ("\nTime to verify results:\n");
   printf ("CPU time = %14.6f seconds\n", cverify);
   printf ("Wall Clock = %14.6f seconds\n\n\n", wverify);

   /* Print out any error messages */
   if (ERRNSOL != 0)
   {
      printf ("*** ERROR: Incorrect number of solutions ***\n\n");
      printf ("\tFound %8d  Expected %8d\n", NSOLS, ERRNSOL);
   }

   if (NERR != 0)
   {
      printf ("*** ERROR: %8d invalid solutions found ***\n", NERR);

      /* Print out up to first 100 invalid solutions */
      lim = NERR;
      if (NERR > 100)
         lim = 100;
      for (i = 0; i < lim; i++)
      {
         printf ("Invalid solution %3d: %8ld   %16lX\n",
                  i+1, ERRORS[i][0], ERRORS[i][1]);
      }
      return;
   }
   else
   {  /* Print out some of the solutions, including the first and last
       * solution.
       * Note that the order of the solutions and thus the answers printed
       * will depend on the value of WIDTH.
       */
      j = NSOLS / 10;
      if (j == 0)
         j = 1;
      printf ("Selected solutions, including the first and last\n\n");
      for (i = 0; i < NSOLS; i += j)
      {
         printf ("%10d  %16lX\n", i+1, SOLS[i]);
      }

      if (NSOLS > 10)
         printf ("%10d  %16lX\n", NSOLS, SOLS[NSOLS- 1]);

      printf ("\nAll solutions valid\n\n");
   }

   return;
}

