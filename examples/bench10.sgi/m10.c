/*
 * PROGRAM m10.c
 *      Benchmark #10  --  N-Queens
 *
 * Count the number of ways N queens can be placed on an N x N
 * chessboard so that no queen can attack another, i.e., no 
 * two queens are on the same row, column, or diagonal.
 *
 * Compute the number of such configurations and for each
 * configuration save the positions of the queens.
 *
 * Output the number of solutions and those positions that
 * are selected by the output routine R10.
 *
 * Despite the fact that obvious symmetries exist on the 
 * chessboard they are to be ignored in this problem.  The 
 * suggested approach is that of tree searching.
 *
 *      Parameter:    N = 16
 *
 *
 * Main Program for Benchmark #10
 *
 * Call time parameters:
 *
 *   N    =  The size of the chess board.
 *               Minimum allowed = 4
 *               Maximum = Default = 16 
 *
 *   WIDTH = The degree of parallelism, the number of branches being
 *           searched at any one time.
 *               Max allowed = 1024
 *               Default = 128
 *
 *   SLVL  = The number of levels of the tree to search before dividing
 *           the resulting near-root nodes among processors.
 *               Minimum allowed = 2
 *               Maximum allowed = 5
 *               Default = 3
 *
 * The default values for WIDTH and SLVL are near optimal for the larger
 * values of N.  However, they can result in very poor performance for 
 * the smaller values of N.  For example, for N = 8, a 2x speedup has
 * been observed using SLVL = 2.  For the larger values of N, some 
 * slight improvement has been observed using SLVL = 4.
 *
 * Not all combinations of these parameters have been tested, and some 
 * may core dump.  Others (particularly for N = 16) may have an overflow 
 * in the space alloted to hold solutions for each processor.  However 
 * the default parameters have been verified for all N > 7.
 * 
 * Command line: m10 [N [WIDTH [sLVL]]]
 */
// *** Includes ***

#include "b10defs.h"

static char cvs_info[] = "BMARKGRP $Date: 2005/01/10 21:17:39 $ $Revision: 1.2 $ $RCSfile: m10.c,v $ $Name:  $";


// *** Definitions ***

#define MASTER 0
#define report(x, y)       if(x == MASTER) y;

#define DEBUG 0

// *** Globals ***

/* An array to hold the solutions */ 
long int * SOLS; 
int N, WIDTH, SLVL; 


// *** Prototypes ***

#if SW_POPCNT
/* Uncomment the following for machines without the popcnt intrinsic.
 * Also uncomment the code for initpc and popcnt in the UTIL routine
 * and the corresponding COMMON statement in P10.
 */
extern void initpc (); 
#endif  /* #if SW_POPCNT */

extern double cputime (), wall ();
extern void c10 (), p10 (), r10 ();



// *** Code ***

int main (int argc, char ** argv)
{
  int np;
  int proc_id;
  
  /* Variables for timing purposes */
  double  cverify, crun, x, y;
  double  wverify, wrun;
  
  /* Array to hold invalid solutions */
  long int ERRORS[100][2];
  int ERRNSOL, NERR, NSOLS;
  
  shmem_init();
  
#if SW_POPCNT
  /* Uncomment the following for machines without the popcnt intrinsic.
   * Also uncomment the code for initpc and popcnt in the UTIL routine
   * and the corresponding COMMON statement in P10.
   */
  initpc (); 
#endif  /* #if SW_POPCNT */
  
  barrier();
  
  np = num_pes();
  proc_id = my_pe();
  
  /* Check command line for valid arguments */
  if (argc >= 2)
    N = atoi (argv[1]);
  else
    N = DFLTSIZE;
  
  if (argc >= 3){
    WIDTH = atoi (argv[2]);
    if(WIDTH != np){
      report(proc_id, printf("WARNING: %d processors specified in argument while there are actually %d processors available\n", WIDTH, np));
    }
  }else
    WIDTH = np;
  
  if (argc >= 4)
    SLVL = atoi (argv[3]);
  else
    SLVL = DFLTSLVL;
  
  /* Get size of chessboard */
  if (N < MINSIZE){
    report(proc_id, printf("<m10> Minimum size for N is %d\n", MINSIZE));
    exit (1);
  }
  
  if (N > MAXSIZE){
    report(proc_id, printf("<m10> Maximum size for N is %d\n", MAXSIZE));
    exit (1);
  }
  
  /* Get number of branches to search in parallel */
  if (WIDTH <= 0)
    WIDTH = DFLTWIDTH;
  
  if (WIDTH > MAXWIDTH){
    report(proc_id, printf ("<m10> Maximum allowed value for WIDTH is %d, WIDTH = %d\n", MAXWIDTH, WIDTH));
    exit (1);
  }
  
  /* Get number of levels of tree to search before dividing
   * resulting near root nodes among processors
   */
  if (SLVL < MINSLVL){
    report(proc_id, printf ("<m10> Minimum allowed value for SLVL is %d, SLVL = %d\n", MINSLVL, SLVL));
    exit (1);
  }

  if (SLVL > MAXSLVL){
    report(proc_id, printf ("<m10> Maximum allowed value for SLVL is %d, SLVL = %d\n", MAXSLVL, SLVL));
    exit (1);
  }
  
  /* Initialize solution and error variables */
  ERRNSOL = NERR = NSOLS = 0;
  
  /* Initialize the timing variables */
  crun = wrun = cverify = wverify = 0.0;
  
  /* Time the actual work being done in subroutine p10 */
  y = wall ();
  x = cputime ();
  x = cputime ();
  
  if(proc_id == 0 && DEBUG)
    printf("Starting Calculations...\n");
  
  p10 (WIDTH, &NSOLS);
  crun = cputime () - x;
  wrun = wall () - y;
  
  /*Check results. Time the check routine c10 */
  y = wall ();
  x = cputime ();
  x = cputime ();
  
  if(proc_id == 0){
    if(DEBUG)
      printf("Confirming Calculations...\n");
    c10 (&NSOLS, &ERRNSOL, &NERR, ERRORS);
  }
  cverify = cputime() - x;
  wverify = wall () - y;
  
  /*Output results */
  if(proc_id == 0){
    if(DEBUG)
      printf("Reporting Calculations...\n");
    r10 (NSOLS, crun, wrun, cverify, wverify, ERRNSOL, NERR, ERRORS);
  }
  
  barrier();
  
  return(0);
}




