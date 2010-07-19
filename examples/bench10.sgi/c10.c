#include "b10defs.h"

/*
 *  This is a check routine for Benchmark #10.  It will verify that the 
 *  number of solutions is correct, and that each solution is valid.  It
 *  will not verify that the solutions are unique.
 *
 *   Parameters:
 *
 *   Provided by calling routine:
 *       SOLS     =  Array holding the solutions (global variable)
 *       NSOLS    =  Number of solutions
 *       N        =  Size of the problem (global variable)
 *
 *   Returned by this routine:
 *       ERRNSOL  =  Holds 0 if correct number of solutions were found;
 *                   otherwise, holds correct number of solutions
 *       NERR     =  Number of invalid solutions found
 *       ERRORS   =  Array to hold first 100 invalid solutions 
 */


static char cvs_info[] = "BMARKGRP $Date: 2005/01/10 21:17:39 $ $Revision: 1.2 $ $RCSfile: c10.c,v $ $Name:  $";


/* Number of solutions to check at a time */
#define MAXTMP     1024

extern int N;
extern long int * SOLS;

/* The correct number of solutions for each parameter size */
int NUMSOL[16] = { 1, 0, 0, 2, 10, 4, 40, 92, 352, 724, 2680, 14200,
                   73712, 365596, 2279184, 14772512 };

/* The following arrays are used in verifying each solution */
long int lut[MAXTMP][16],
         row[MAXTMP],
         diag1[MAXTMP],
         diag2[MAXTMP],
         err[MAXTMP];
      
void c10 (int *NSOLS, int *ERRNSOL, int *NERR, long int ERRORS[100][2])
{
   register long int id1, id2, irow, isc, ival;
   register int i, j, k, len;

   /* Initialize error arrays */
   *ERRNSOL = 0;
   for (i = 0; i < 100; i++)
   {
      ERRORS[i][0] = 0;
      ERRORS[i][1] = 0;
   }

   /* Verify that the correct number of solutions have been found.  If
    * not, store the expected number of solutions in ERRNSOL
    */
   if (*NSOLS != NUMSOL[N - 1])
      *ERRNSOL = NUMSOL[N - 1];

   /* Set up to verify each solution */
   *NERR = 0;
   for (i = 0; i < N; i++)
   {
      for (j = 0; j < MAXTMP; j++)
      {
          lut[j][i] = 1L << (i + 16);
      }
   }

   /* For MAXTMP solutions at each pass */
   for (i = 0; i < *NSOLS; i += MAXTMP)
   {
      len = MAXTMP;
      if (i + MAXTMP > *NSOLS)
         len = *NSOLS - i;
      for (j = 0; j < len; j++)
      {  /* Use the first position to initialize the occupied row and two
          * diagonal histories
          */
         ival = 15L & SOLS[i + j];
         irow = lut[j][ival];
         row[j] = irow;
         diag1[j] = irow;
         diag2[j] = irow;
         err[j] = 0;
      }

      /* For each subsequent position in the solution ... */
      for (k = 1; k <= N-1; k++)
      {
         isc = k * 4;
         for (j = 0; j < len; j++)
         {
            /* ... verify no conflict in the row and the two diagonals. */
            ival = 15L & (SOLS[i + j] >> isc);
            irow = lut[j][ival];
            id1 = irow << k;
            id2 = irow >> k;
            err[j] = err[j] | (irow & row[j]) + (id1 & diag1[j]) +
                     (id2 & diag2[j]);

            /* Add in this row and diagonals to the histories */
            row[j] = irow | row[j];
            diag1[j] = id1 | diag1[j];
            diag2[j] = id2 | diag2[j];
         }
      }

      /* If an error is found, update NERR and ERRORS array.  Only the
       * first 100 errors will be recorded.
       */
      for (j = 0; j < len; j++)
      {
         if (err[j] != 0)
         {
            if (*NERR < 100)
            {
               ERRORS[*NERR][0] = j + i;
               ERRORS[*NERR][1] = SOLS[j + i];
            }
            *NERR++;
         }
      }
   }

   return;
}

