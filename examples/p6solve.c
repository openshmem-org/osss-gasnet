#include "bench6.h"

static char cvs_info[] = "BMARKGRP $Date: 2005/01/10 21:17:26 $ $Revision: 1.2 $ $RCSfile: p6solve.c,v $ $Name:  $";


/* use macros to address multi-dim arrays with variable dim's like Fortran */
#define A(I,J)		A[ (J) + nw * (I) ]

#ifdef SW_POPCNT
extern int popcnt();
#endif /* SW_POPCNT */

void
solve(	uint64 * eqs,	/* eqs[neqs][nw] == eqs(m,*,*) - m-th set of eqs */
	uint64 * x,	/* x[nw] == sols(m,*) - returns solution */
	uint64 * A,	/* A[neqs*nw] - copy equations here to work on */
	int neqs,	/* msize, number of equations */
	int nw,		/* eqnw, # of wds to store row of A (plus bit for c)*/
        int bs,		/* block size for block reduction */
	int * indep)	/* set = 0 if matrix is singular, 1 if nonsing */
{
/*------------------------------------------------------------------------------
*  Solves the mod-2 linear system Ax = c, where A is (neqs x neqs) bits,
*  c and x are column vectors of neqs bits.
*  This routine assumes that the rows of A are packed into words and
*  augmented at the end (bit # neqs) by the c(i) value.
*  x is returned as a packed bit vector.
*  A is copied from eqs into a work area.
*
*  nw is the number of words in a row of the A matrix, including
*  the c value at the end of the row (i.e. it will require an extra
*  word if neqs is a multiple of the word size.
*
*  bs is the block size for block reduction to speed up the solution.
*--------------------------------------------------------------------------*/

   int kary[MSIZE0];	/* b bits of each (active) row of A */
   int flags[TWOB];	/* flags[i]=1 if i in span of curr. subbasis of kary */
   int tblent[TWOB];	/* rows(i) has kary = tblent[i] */
   uint64 rows[TWOB][EQNW0]; /* where we construct 2^b rows w. distinct kary */
   uint64 rsrt[TWOB][EQNW0]; /* sort rows into rsrt by kary using tblent */
   int i, j;
   int b, twob, istart, rownum, ieq, newk, nextpow2, ii, newrow, ite;
   int tmpi, v;
   uint64 twobmask, tmp, xi;
   int iwrd, ibit, cwrd, cbit, cnst, xsum;

   const int	ws	= WS;
   const int	logws	= LOGWS;
   const int	wsmask	= WSMASK;

   /* copy equations to work array A */
   for ( i = 0; i < neqs*nw; ++i )
      A[i] = eqs[i];

   /* zero out solution vector x and first entry in rows and rsrt */
   for ( i = 0; i < nw; ++i )
   {
      x[i] = 0;
      rows[0][i] = 0;
      rsrt[0][i] = 0;
   }

   b = bs;

   for ( istart = 0; istart < neqs; istart += b )
   {
      /* reduce on cols istart thru istart+b-1 */

      if ( (istart+bs) > neqs )
         b = neqs - istart;  /* last iteration */
      twob = 1 << b;
      twobmask = twob - 1;
      /* initialize flags for this block */
      flags[0] = 1;
      for ( i = 1; i < twob; ++i )
         flags[i] = 0;

      /* for each row >= istart, set kary[j] = row<b,istart> */
      
      iwrd = istart >> logws;
      ibit = istart & wsmask;
      if ( (ibit+b) <= ws )
      {
         /* pick bits from one word */
         for ( j = istart; j < neqs; ++j )
            kary[j] = (A(j,iwrd) >> ibit) & twobmask;
      }
      else
      {
         /* pick bits from 2 adjacent words */
         for ( j = istart; j < neqs; ++j )
            kary[j] =
               ( (A(j,iwrd) >> ibit) | (A(j,iwrd+1) << ws-ibit) ) & twobmask;
      }

      /* look for row not in span of prev rows */
      rownum = istart;
      for ( ieq = 0; ieq < b; ++ieq )
      {
         while ( flags[kary[rownum]] == 1 )
         {
            /* kary is in the span of previously chosen rows */
            rownum += 1;
            if ( rownum == neqs )
            {
	       /* can't find enough indep rows */
               *indep = 0;
               goto end;
            }
         }
         /* found a new indep row, not in span of prev rows */
         newk = kary[rownum];
         flags[newk] = 1; 
         nextpow2 = 1 << ieq; /* 2^ieq */
         /* rows[nextpow2] = row rownum of A */
         for ( j = 0; j < nw; ++j )
            rows[nextpow2][j] = A(rownum,j);
         /* swap rows istart+ieq and rownum, including their kary values */
         ii = istart+ieq;
         if ( ii != rownum )
         {
            for ( j = 0; j < nw; ++j )
            {
               tmp         = A(ii,j);
               A(ii,j)     = A(rownum,j);
               A(rownum,j) = tmp;
            }
            tmpi         = kary[ii];
            kary[ii]     = kary[rownum];
            kary[rownum] = tmpi;
         }
         tblent[nextpow2] = newk;
         /* xor new row with all previous nonzero rows for new span */
         for ( v = 1; v < nextpow2; ++v )
         {
            newrow = nextpow2 + v;
            ite = tblent[v] ^ newk;
            flags[ite] = 1;
            tblent[newrow] = ite;
            for ( j = 0; j < nw; ++j )
               rows[newrow][j] = rows[nextpow2][j] ^ rows[v][j];
         }
      }  /* end for(ieq...) */

      /* now we have b indep rows */
      /* sort 2^b rows to rsrt by indices in tblent */
      /*   so rsrt[v] has kary = v */
      for ( i = 1; i < twob; ++i )
         for ( j = 0; j < nw; ++j )
            rsrt[tblent[i]][j] = rows[i][j];

      /* now reduce eq's */

      /* the b rows beginning at istart get new 1's on diagonal */
      /* the easiest way is to use rsrt[1], rsrt[2], rsrt[4], etc */
      /* so the new b x b subblock is diagonal, not just upper triangular */
      for ( i = 0; i < b; ++i )
         for ( j = 0; j < nw; ++j )
            A(istart+i,j) = rsrt[1<<i][j];

      /* reduce block of b cols on rows below */
      for ( i = istart+b; i < neqs; ++i )
         for ( j = 0; j < nw; ++j )
            A(i,j) = A(i,j) ^ rsrt[kary[i]][j];

   } /* end for loop on istart */

   /*-------------------------------------------------------------------
   *  A is now upper triangular with ones on diag.  Now back substitute
   *  to fill up solution vector x from bit (neqs-1) backwards to 0
   *------------------------------------------------------------------*/
   cwrd  = neqs >> logws;
   cbit  = neqs & wsmask;

   for ( i = neqs-1; i >= 0; --i )
   {
      iwrd = i >> logws;
      ibit = i & wsmask;
      cnst = (A(i,cwrd) >> cbit) & (uint64)1;

      xsum = 0;
      for ( j = iwrd; j < nw; ++j )  /* preceding words all 0 */
      {
         xsum += popcnt( A(i,j) & x[j] );
      }
      xi = cnst ^ (xsum & 1);  /* next bit in x vector */
      x[iwrd] |= (xi << ibit);
   }
   *indep = 1;

 end:
   return;
}


/*============================================================================*/

void
setupeqs ( uint64 * stream, uint64 * eq, int wds, uint64 bits, int nshft )
{
/*
 * shifts msize+1 bits from stream into equation array
 * msize+1 fills wds full words; if bits is not 0, it's a mask for R-most bits
 * nshft slides around as we fill up one eq after another
 */
   int i;

   if ( nshft  ==  0 )
   {
      for ( i = 0; i < wds; ++i )
         eq[i] = stream[i];

      if ( bits != 0 )
         eq[wds] = bits & stream[wds];
   }
   else
   {
      for ( i = 0; i < wds; ++i )
         eq[i] = (stream[i] >> nshft) | (stream[i+1] << 64-nshft);

      if ( bits != 0 )
         eq[wds] =
            bits & ( (stream[wds] >> nshft) | (stream[wds+1] << 64-nshft) );
   }
}
