/*   Benchmark #10  --  N Queens
 *   PARAMETERS
 *   Provided by calling routine:
 *   N      =   Number of squares on a side of the chessboard
 *   WIDTH  =   Number of "processors" working on the problem
 *   SLVL   =   Number of levels of the tree to search before dividing the
 *              resulting near-root nodes among processors
 *    Returned by this routine:
 *   NSOLS  =   Number of solutions found
 *   SOLS   =   Array storing the solutions found
 *    Scratch Arrays:
 *   MSK    =   Masks for each level on each processor indicating which
 *              squares remain to be tried from that level's current node
 *   RANDDS =   Array which has for each level of each processor, a word
 *              containing 3 masks indicating which rows and diagonals
 *              are empty
 *   W      =   A list of the near-root nodes
 *   WW     =   Array used to store intermediate nodes for the
 *              computation of W
 *   STKPTR =   Array of stack pointers, one per processor; the pointer 
 *              to which node is currently being worked on          
 *   LUT    =   A lookup table for each processor to determine which rows
 *              and diagonals are used by a square
 *   SOLUT  =   For each processor, the number of solutions found on that
 *              processor
 *
 *  Basic Algorithm
 *    As suggested, a tree search approach is used.  The tree consists of
 * all the solutions and partial solutions of placing N queens on the
 * NxN board.  The tree is constructed by assuming that you get from a
 * node at level (K) to a node at level (K+1) by adding one queen to a
 * legal square in column (K+1) on the board.  Thus the level of a 
 * node is equal to the number of queens placed in order to reach that 
 * node.  The root node is at level 0, solution nodes are at level N.
 * Each node contains information about which queens have been placed
 * so far.  A node at level J is a leaf node if it is illegal to
 * place a queen in any of the N squares of the column J+1, given the
 * positions of the queens in columns 1 through J.
 *
 *    The algorithm is based on a SIMD programming model, but works
 * well on other architectures as well.  In this model, each "processor"
 * selects a node near the root of the tree of partial solutions
 * and works on the sub-tree based on that node.  It does a depth-first 
 * search of the sub-tree.  Whenever any processor finishes its sub-tree,
 * it starts on a new near-root node which is not part of any other 
 * processor's sub-tree, or if none exists, it splits a node with another 
 * processor.  Because each processor does a depth first search, it has 
 * to store at most N nodes, as this is the depth of the tree. The N 
 * nodes can be thought of as a stack.  The stack pointer determines not
 * only the depth of the stack, but also the column of the board which
 * is currently being worked on.  At each level of the stack, 4 masks
 * are stored:
 * 1) a mask indicating which of the rows are empty;
 * 2) a mask indicating which of the N nw-se diagonals of this column
 *    are empty;
 * 3) a mask indicating which of the N ne-sw diagonals of this column
 *    are empty;
 * 4) a mask, initially the AND of the above masks, indicating the
 *    rows in this column in which a queen may be tried (this will
 *    be referred to as the ok-mask).
 *
 *    The main loop of the program manipulates these 4 masks, as well as the
 * stack pointers of each processor.  It gets the current stack pointer
 * and uses it to fetch the ok-mask.  The first bit set in this
 * mask indicates the first valid square in this column (i.e. the first 
 * valid node at this level) which has not yet been tried.  This square 
 * is selected and the corresponding bit in the ok-mask is turned off.  
 * The masks of the row and diagonals for the current stack level are
 * copied to the next stack level, but with the selected square's row 
 * and diagonals marked as occupied.  In copying the masks of the
 * diagonals, they must be shifted by one place, because the interesting 
 * diagonals change with the change in level.  The ok-mask for this new
 * stack level is then generated from the AND of the other masks.  The
 * stack pointer is incremented if the square selected is valid, or
 * decremented if no valid squares remain.  If incremented and it
 * reaches the lowest depth, this must be a full solution, and the
 * processor solution count is incremented.  Any processor that is done
 * with its work will be given more.
 *
 *    Modifications to basic algorithm:
 *
 *   The masks for the row and diagonals are packed into one word.
 * This reduces the number of gathers and scatters needed.  It also
 * permits the marking of the row and diagonals as occupied to be done 
 * with one table lookup, as opposed to three variable shifts.  The
 * lookup table is duplicated on each "processor" to avoid hot spots.
 * In order for there to be enough work for many processors, it may be
 * necessary for the so-called near-root nodes to be at the third 
 * or forth level.  This starting level is determined by the
 * variable SLVL.
 */

// *** Includes ***

#include "b10defs.h"

static char cvs_info[] = "BMARKGRP $Date: 2005/01/10 21:17:39 $ $Revision: 1.2 $ $RCSfile: p10.c,v $ $Name:  $";


// *** Definitions ***

/* Maximum size for N */
#define MN    MAXSIZE

/* Maximum size for WIDTH */
#define MWDTH MAXWIDTH

/* Maximum size for SLVL */
#define MSLVL MAXSLVL

#define MN_POWER_5  1048576
#define MN_POWER_4  65536

#ifdef INTRINSICS
#define popcnt    _popcnt
#endif

#if SW_POPCNT
   /* Uncomment the following for machines without the popcnt intrinsic.
    * Also, uncomment the code for initpc and popcnt in the UTIL routine
    * and the call to initpc in M10.
    */
extern int popcnt ();
#endif  /* #if SW_POPCNT */



// *** Globals ***

/* The array of stacks of ok-mask indicating squares where a queen can be
 * placed but which haven't been tried already.
 */
long int MSK[MWDTH][MN],
         HIST[MWDTH][MN];

/* The array of stacks of triple masks showing which rows and which
 * ne-sw and nw-se diagonals are already occupied.
 */
long int RANDDS[MWDTH][MN];

/* Number of complete solutions found by each processor */
long int SOLUT;
#define MAXTS    23000000
long int TSOLS[MAXTS];

/* The "stack pointers", one per "processor" */
int STKPTR[MWDTH];

/* Table of nodes near the root of the tree for beginning points in a 
 * processor (MN ** MSLVL = 1048576)
 */
long int W[MN_POWER_5],
         H[MN_POWER_5];
long int WW[MN_POWER_4],
         HH[MN_POWER_4];
/* Number of direct children nodes not yet searched from the sub-tree root
 * on each "processor"
 */
long int LEFT[BITSIZ];

/* Lookup table with entries having bits clear marking which rows and
 * diagonals will be occupied for each square in a column.  One copy
 * of table for each "processor"
 */
long int LUT[MWDTH][BITSIZ];

extern int N, WIDTH, SLVL;
extern long int * SOLS;
unsigned long totalSolutions;




// *** Code ***

void p10 (int WDTH, int *NSOLS)
{
   register int i, j, k, kk, IW, KW, SL;
   register long int BIT, ZAPBIT, HALF, HMSK, ISP;
   register long int M, NISP, NRDD, RDD, RD2MSK, SC, SMASK;
   long int D1MSK, D2MSK, D1MSZ, D2MSZ, DMSK, LUT1, LUT2;
   long int NMASK, MASK1, RMSK, RMSZ, WMSK;
   int proc_id = my_pe();
   int done = 0;
   unsigned long step; 
   long startSubtree, endSubtree;
   
   D2MSZ = BITSIZ / 3;
   /* Max size of mask for ne-sw diagonal */
   D1MSZ = (BITSIZ - D2MSZ) / 2;
   /* Max size of mask for rows */
   RMSZ = BITSIZ - D2MSZ - D1MSZ;
   /* Mask for rows */
   RMSK = (1L << N) - 1;
   DMSK = (1L << (N - 1)) - 1;
   /* Mask for nw-se diagonals */
   D2MSK = DMSK << (RMSZ + D1MSZ);
   /* Mask for nw-se diagonals and rows */
   RD2MSK = RMSK | D2MSK;
   /* Mask for ne-sw diagonals */
   D1MSK = DMSK << (RMSZ + 1);
   MASK1 = (1L << (RMSZ + D1MSZ)) + (1 << RMSZ) + 1;
   /* Mask for marking as unoccupied the new diagonals for next column */
   SMASK = (1L << (RMSZ + D1MSZ)) + (1L << (RMSZ + (N - 1)));

   for (i = 0; i < WDTH; i++)
   {  /* Just in case... */
      MSK[i][0] = 0;
   }
   SOLUT = 0;
   

   /* Generate a lookup table for each "processor" */
   for (i = 0; i < N; i++)
   {
      for (j = 0; j < WDTH; j++)
      {
         LUT[j][i] = -1L - (MASK1 << i);
      }
   }
   for (i = N; i < BITSIZ; i++)
   {
      for (j = 0; j < WDTH; j++)
      {
         LUT[j][i] = 0;
      }
   }

   *NSOLS = 0;
   NMASK = (1L << N) - 1;
   k = 0;

   /* Generate nodes close to root to serve as roots for sub-trees */

   /* For each square in the first column... */
   for (i = 0; i < N; i++)
   {
      LUT1 = LUT[1][i];

      /* ... generate the masks as they would look in the second column */
      LUT1 = (RD2MSK & LUT1) + ((D1MSK & LUT1) >> 1) + (D2MSK & LUT1) + SMASK;

      /* For each square in the 2nd column not conflicting with first queen,
       * generate the masks as they would look in the third column.
       *
       * First, the rows before the queen in the first column
       */
      for (j = 0; j <= i - 2; j++)
      {
         LUT2 = LUT1 & LUT[1][j];
         W[k] = (RD2MSK & LUT2) + ((D1MSK & LUT2) >> 1) + (D2MSK & LUT2) + SMASK;
         H[k] = (i << 4) + j;
         k++;
      }

      /* Then the rows after the queen in the first column */
      for (j = i + 2; j < N; j++)
      {
         LUT2 = LUT1 & LUT[1][j];
         W[k] = (RD2MSK & LUT2) + ((D1MSK & LUT2) >> 1) + (D2MSK & LUT2) + SMASK;
         H[k] = (i << 4) + j;
         k++;
      }
   }

   SL = 2;
   while (SL < SLVL)
   {
      kk = k;
      k = 0;
      for (i = 0; i < kk; i++)
      {
         WW[i] = W[i];
         HH[i] = H[i];
      }

      for (i = 0; i < N; i++)
      {
         for (j = 0; j < kk; j++)
         {
            WMSK = (WW[j] & (WW[j] >> 22)) & (WW[j] >> 43);
            if ((WMSK & (1L << i)) != 0)
            {
               LUT1 = WW[j] & LUT[1][i];
               W[k] = (RD2MSK & LUT1) + ((D1MSK & LUT1) >> 1) + (D2MSK & LUT1) + SMASK;
               H[k] = (HH[j] << 4) + i;
               k++;
            }
         }
      }
      SL++;
   }
   
   
   
   if (WDTH < k)
      WIDTH = WDTH;
   else
      WIDTH = k;
   KW = k;

   if(proc_id >= WIDTH){
     goto label_end;
   }

   step = KW / WIDTH;
   if(KW % WIDTH)
     step++;
   
   startSubtree = step * proc_id;
   endSubtree = startSubtree + step;

   if(endSubtree > KW) 
     endSubtree = KW; 
   if(startSubtree >= KW){
     goto label_end;
   }
   
   RANDDS[proc_id][1] = W[startSubtree];
   MSK[proc_id][1] = (W[startSubtree] & (W[startSubtree] >> 22)) & (W[startSubtree] >> 43);
   HIST[proc_id][1] = H[startSubtree];
   
   STKPTR[proc_id] = 1;
   

   /* The number of nodes used so far is the # of processors */
   IW = WIDTH;
   *NSOLS = 0;


   /* Repeat until no sub-tree has any direct child nodes which are not being
    * worked upon
    */
   
   
   /* Clear number of processors which have run out of work */
   while(!done){
     
     ISP = STKPTR[proc_id];
     
     /* Get the ok-mask from the proper stack level */
     M = MSK[proc_id][ISP];
     
     /* Find the first square not yet processed */
     ZAPBIT = M & -M;
     
     /* Get the ordinal of that square */
     BIT = popcnt (ZAPBIT - 1);
     
     /* Update the history */
     HIST[proc_id][ISP + 1] = (HIST[proc_id][ISP] << 4) + BIT;
     
     /* Mark this square as being processed and store updated mask */
     MSK[proc_id][ISP] = M ^ ZAPBIT;
     
     /* For next column, mark the row and diagonals of this square occupied */
     RDD = LUT[proc_id][BIT] & RANDDS[proc_id][ISP];
     
     /* If first bit was a valid bit, increment stack pointer; otherwise
      * this node has been fully processed, so decrement stack pointer
      */
     NISP = ISP + 1 - (2L & ((N - 1 - BIT) >> 62));
     
     /* Calculate new diagonal masks as seen from the next column by
      * shifting them and adding as unoccupied the new diagonals first
      * seen from that column.
      */
     NRDD = (RD2MSK & RDD) + ((D1MSK & RDD) >> 1) + (D2MSK & RDD) + SMASK;
     
     /* Store the row mask and the diagonal masks  */
     RANDDS[proc_id][ISP + 1] = NRDD;
     
     /* Calculate the ok-mask for the next column */
     MSK[proc_id][ISP + 1] = (NRDD & (NRDD >> 22)) & (NRDD >> 43);
     
     /* If the next column is past the last, this is a full solution ... */
     
     if (NISP > (N - SLVL)){
       TSOLS[SOLUT] = HIST[proc_id][ISP + 1];
       SOLUT++;
       /* ... and the stack can be decremented */
       NISP--;
     }
     
      /* Store the new stack pointer */
     STKPTR[proc_id] = NISP;
     
     /* If the processor finished this sub-tree, find the next... */
     if (NISP < 1){
       
       /* Set-up this pe for its next sub-tree to calculate... */  
       done = 1;
       
       startSubtree++;
       if(startSubtree < endSubtree){
	 RANDDS[proc_id][1] = W[startSubtree];    
	 MSK[proc_id][1] = (W[startSubtree] & (W[startSubtree] >> 22)) & (W[startSubtree] >> 43);    
	 HIST[proc_id][1] = H[startSubtree];    
	 
	 /* Put the node on the stack */    
	 STKPTR[proc_id] = 1;   
	 done = 0;
       }
       
     }
   } /* end for( ; ; )  */
   /* Continue with new stack frames */
   
 label170:
   /* Processor is finished */
   
   done = 0;

   while(!done){
     ISP = STKPTR[proc_id];
     M = MSK[proc_id][ISP];
     ZAPBIT = M & -M;
     BIT = popcnt (ZAPBIT - 1);
     HIST[proc_id][ISP + 1] = (HIST[proc_id][ISP] << 4) + BIT;
     MSK[proc_id][ISP] = M ^ ZAPBIT;
     RDD = LUT[proc_id][BIT] & RANDDS[proc_id][ISP];
     NISP = (ISP + 1) - (2L & ((N - 1 - BIT) >> 62));
     NRDD = (RD2MSK & RDD) + ((D1MSK & RDD) >> 1) + (D2MSK & RDD) + SMASK;
     RANDDS[proc_id][ISP + 1] = NRDD;
     MSK[proc_id][ISP + 1] = (NRDD & (NRDD >> 22)) & (NRDD >> 43);
     
     if (NISP > (N - SLVL)){
       TSOLS[SOLUT] = HIST[proc_id][ISP + 1];
       SOLUT++;
       NISP--;
     }
     
     /* Once a processor has finished, don't change its stack pointer */
     if (ISP >= 1)
       STKPTR[proc_id] = NISP;
     else
       done = 1;
   }

 label_end:
   
   totalSolutions = 0;
   
   {
     static long flag = 0;
     
     if(proc_id == 0)
       flag = 1;
     
     barrier();
     
     shmem_wait(&flag, 0);
     
     shmem_get(&step, &totalSolutions, 1, 0);
     totalSolutions = step + SOLUT;
     shmem_put(&totalSolutions, &totalSolutions, 1, 0);
     
     if(proc_id+1 < WIDTH)
       shmem_long_swap(&flag, 1, proc_id+1);
   }
   
   barrier();

   /* Copy all of my solutions onto the Master's solution array */
   if(proc_id > 0)
     shmem_put(TSOLS + step, TSOLS, SOLUT, 0);

   barrier();
   
   *NSOLS = totalSolutions;
   SOLS = TSOLS;
	     
   return;
}

