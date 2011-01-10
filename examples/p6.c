#include "bench6.h"

static char cvs_info[] = "BMARKGRP $Date: 2005/01/10 21:17:26 $ $Revision: 1.2 $ $RCSfile: p6.c,v $ $Name:  $";


/* syymetric data so shmem op's can access it */
int matchinfo;
long pSync[_SHMEM_BCAST_SYNC_SIZE];

/* use macros to address multi-dim arrays with variable dim's like Fortran */
#define eqs(I,J,K)	eqs[ (K) + eqnw * ( (J) + (msize * (I)) ) ]
#define sols(I,J)	sols[ (J) + (eqnw * (I)) ]


#ifdef SW_POPCNT
extern int popcnt();
#endif /* SW_POPCNT */
extern void solve();
extern void setupeqs();

void
p6 (	uint64 * s,		/* s        [snw] */
	uint64 * startbits,	/* startbits[snw] */
	uint64 * eqs,		/* eqs   [maxmatch][msize][eqnw] */
	uint64 * sols,		/* sols  [maxmatch][eqnw] */
	int * starts,		/* starts[maxmatch] */
	int * indep,		/* indep [maxmatch] */
	int slen, int snw, int eqnw, int msize, int plen,
	uint64 care_and_mask, uint64 care_xor_mask,
#ifdef Timer
	int loop, int nloops,
#endif
	int mype, int npes, int mpes,
	int * matches, int * gmatches, int * nindep )
{
/* work is used in eliminate to hold shifted s array, */
/*   then used here to retrieve partial startbits from all pe's */
/*   then used to hold the equations in the solve subr */
   uint64 work[SNWMAX];		/* work[snw] */

   uint64 clearbit, AndMask, XorMask, ckpattern;
   int match, toofar, bitpos, matlen, mwds, mbits;
   uint64 test, bits;
   int i, j, m, nlevels, mpesx, mylevel, mypex, lev;
   int indxs, windxs, bindxs;
   int ntasks, mystart, mystop, extra;

   const int	ws	= WS;
   const int	logws	= LOGWS;
   const int	wsmask	= WSMASK;

#ifdef Timer
   double cputime(), x0, x1;
   static double matcht=0, match0t=0, combinet=0, solvet=0;
#endif

   for (i=0; i < _SHMEM_BCAST_SYNC_SIZE; i++)
      pSync[i] = _SHMEM_SYNC_VALUE;
   shmem_barrier_all();

/*------------------------------------------------------------------------------
*  (1) find all occurrences of the pattern in the bit stream
*--------------------------------------------------------------------------*/

#ifdef Timer
/* start timing for pattern-matching */
   x0 = cputime();
   x1 = cputime();
#endif

/*-------------------------------------------------------------------------
*  round-robin the bit positions to check for pattern-start in s
*  this zeroes all bits in position 'i' in startbits that are not starts
*--------------------------------------------------------------------------*/

if ( mype < mpes )
{
#ifdef Debug
   if ( mype == 0 ) printf("pattern match\n");
#endif
   /* shift s array into work array, offset by 32 bits */
   /* we check for pattern at pos 0-31 in 's', at pos. 32-63 in 'work' */
   for ( i = 0; i < snw-1; ++i )
      work[i] = ( s[i] >> 32 ) | ( s[i+1] << 32 );
   work[snw-1] = s[snw-1] >> 32;

   for ( j = mype; j < 64; j += mpes )
   {
      clearbit = ~ ( (uint64)1 << j );

      if ( j < 32 )
      {
         AndMask = care_and_mask << j;
         XorMask = care_xor_mask << j;
         for ( i = 0; i < snw; ++i )
         {
            ckpattern = ( s[i] ^ XorMask ) & AndMask;
            if ( ckpattern != 0 )
               startbits[i] &= clearbit;  /* zeroize bit */
         }
      }
      else  /* 32 <= j < 64 */
      {
         AndMask = care_and_mask << j-32;
         XorMask = care_xor_mask << j-32;
         for ( i = 0; i < snw; ++i )
         {
            ckpattern = ( work[i] ^ XorMask ) & AndMask;
            if ( ckpattern != 0 )
               startbits[i] &= clearbit;  /* zeroize bit */
         }
      }
   }

#ifdef Timer1
   x1 = cputime() - x1;
   match0t += x1;
#endif

/*--------------------------------------------------------------------------
*  All PE's have checked their own bit positions in the startbit array.
*  Now AND together all the results, by setting up a tree of PEs, passing
*  results and ANDing on the way up to root node.
*--------------------------------------------------------------------------*/
   if ( mpes > 1 )
   {
#ifdef Timer
      x1 = cputime();
#endif

      /* set up tree of pe's */
      nlevels = 0;
      mpesx   = mpes;
      while ( mpesx != 0 )
      {
         ++nlevels;
         mpesx = mpesx >> 1;
      }

      mylevel = -1;
      mypex   = mype + 1;
      while ( mypex != 0 )
      {
         ++mylevel;
         mypex   = mypex >> 1;
      }

      /* be careful if mpes < global npes */
      /* shmem_barrier_all(); */
      shmem_barrier(0,0,mpes,pSync);

      /* pull startbits one level up tree from (2*mype+1, 2*mype+2) */
      /*   where it's received as "work" and AND'ed with startbits there */

      for ( lev = nlevels - 1; lev >= 1; --lev )
      {
         if ( (mylevel == lev-1) && (mpes > (2*mype+1)) )
         {
            shmem_get64 ( work, startbits, snw, (2*mype+1) );
            for ( i = 0; i < snw; ++i )
               startbits[i] &= work[i];
         }

         if ( (mylevel == lev-1) && (mpes > (2*mype+2)) )
         {
            shmem_get64 ( work, startbits, snw, (2*mype+2) );
            for ( i = 0; i < snw; ++i )
               startbits[i] &= work[i];
         }

         shmem_barrier(0,0,mpes,pSync);
      }
#ifdef Timer1
      x1 = cputime() - x1;
      combinet += x1;
#endif
   }  /* if mpes > 1 */

   shmem_barrier(0,0,mpes,pSync);

#ifdef PrintBits
   if ( (slen < 10000) && (mype == 0) )
   {
#ifdef CRAY
      for (i=0;i<snw;++i) printf("%064B\n", s[i]);
      printf("\n");
      for (i=0;i<snw;++i) printf("%064B\n", startbits[i]);
      printf("\n");
#else
      int j;  uint64 word;
      for (i=0; i<snw; ++i)
      {
         word = s[i];
         for ( j = 63; j >= 0; j-- )
            printf( "%d", (int) ( (word >> j) & (uint64)1 ) );
         printf("\n");
      }
      printf("\n");
      for (i=0; i<snw; ++i)
      {
         word = startbits[i];
         for ( j = 63; j >= 0; j-- )
            printf( "%d", (int) ( (word >> j) & (uint64)1 ) );
         printf("\n");
      }
      printf("\n");
#endif
   }
#endif  /* PrintBits */

} /* if mype < mpes --- end pattern match using only mpes PEs */

/*----------------------------------------------------------------------------*/

/* PE0 has the right startbits; record the locations, adding plen to each */
   if ( mype == 0 )
   {
      match = 0;
      for ( i = 0; i < snw; ++i )
      {
         if ( startbits[i] != 0 )
         {
            test = startbits[i];
            while ( test != 0 )
            {
               /* # of trailing 0's = bitpos of rightmost 1 in word */
               bitpos = popcnt( test ^ (test-1) ) - 1;
               starts[match] =  ws * i + bitpos + plen;
               ++match;
               /* remove R-most bit=1; in rare case >1 match in word, go again */
               test ^= ((uint64)1 << bitpos);
            }
         }
      }
      *matches = match;

      /* mark and skip starts that have too few stream bits to form equations */
      matlen = msize * (msize+1);  /* m*m a's, m b's needed to form eq's */
      toofar = 0;

      for ( m = *matches-1; m >= 0; --m )
      {
         if ( (starts[m] + matlen) > slen )
         {
            toofar += 1;
            indep[m] = -1;
         }
         else
            break;
      }

      /* only work on starts with full equations */
      *gmatches = *matches - toofar;

      matchinfo = *matches + (*gmatches << 16);
#if 0
      printf("matches,gmatches = %d  %d\n", *matches,*gmatches);
      for (m=0; m<*gmatches; ++m) printf("%d\n", starts[m]);
#endif
   } /* end if mype==0 */

/*----------------------------------------------------------------------------*/
/* PE0 broadcasts starts array to other PEs */
   if ( npes > 1 )
   {
      shmem_barrier_all();

      shmem_broadcast32 ( &matchinfo, &matchinfo, 1, 0, 0, 0, npes, pSync );

      if ( mype > 0 )
      {  *matches  = matchinfo & 0xFFFF;
         *gmatches = matchinfo >> 16;
      }

      shmem_broadcast32 ( starts, starts, *gmatches, 0, 0, 0, npes, pSync );
   }

#ifdef Timer
/* stop timing for pattern-matching */
   x0 = cputime() - x0;
   matcht += x0;
#endif

/*--------------------------------------------------------------------------
* distribute sets of equations in even chunks among the PE's
* (it works ok even if npes>gmatches & ntasks=0)
*--------------------------------------------------------------------------*/
   ntasks  = *gmatches / npes;
   extra   = *gmatches - ntasks * npes;
   mystart =  mype    * ntasks     + min ( mype, extra );
   mystop  = (mype+1) * ntasks - 1 + min ( mype+1, extra );

   *nindep = 0;

/*--------------------------------------------------------------------------
* (2) solve equations in evenly divided chunks among the PE's
*--------------------------------------------------------------------------*/
   mwds  = (msize+1) >> logws;
   mbits = (msize+1) & wsmask;
   bits  = RMASK(mbits);

#ifdef Timer
   x0 = cputime();
#endif

   for ( m = mystart; m <= mystop; ++m )
   {
      /* for m-th pattern-match, set up equations in array "eqs" */
      indxs = starts[m];
      for ( j = 0; j < msize; ++j )
      {
         /* word and bit position in s, to start sliding into eqs */
         windxs = indxs >> logws;
         bindxs = indxs & wsmask;
         setupeqs( &s[windxs], &eqs(m,j,0), mwds, bits, bindxs );
         /* advance msize+1 bits - that's a row of a's and one b */
         indxs += msize + 1;
      }

#ifdef Debug
      printf("call solve %d\n", m);
#endif

/* solve the m-th set of equations and if indep, return m-th solution in sols */

      solve ( &eqs(m,0,0), &sols(m,0), work, msize, eqnw, BS, &indep[m] );

      *nindep += indep[m];

   }  /* end loop on m in my chunk */

#ifdef Timer
   x0 = cputime() - x0;
   solvet += x0;

   if ( loop == nloops )
   {
      if (mype < mpes)
      printf("%d  matcht    %f\n", mype, matcht);
/*
      printf("%d  match0t   %f\n", mype, match0t);
      printf("%d  combinet  %f\n", mype, combinet);
      printf("%d  solvet    %f\n", mype, solvet);
*/
   }
#endif

}  /* p6 */
