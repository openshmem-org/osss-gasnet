/************************************************************************/
/*File:          prandom.c						*/
/*Date Created:  Summer 1999						*/
/*Current Date:  July 10, 2001						*/
/*Description:   C translation of FORTRAN code for the random number	*/
/*               generator used by the Benchmarks			*/
/************************************************************************/

/*Prototypes for functions in this file are found in the following     */
/*header file.                                                         */
#include "prandom.h"
#include <stdio.h>

static char cvs_info[] = "BMARKGRP $Date: 2005/01/10 21:17:26 $ $Revision: 1.2 $ $RCSfile: prandom.c,v $ $Name:  $";

/*************The following values are defined in prandom.h**************/
/*		#define L 158						*/
/*		#define k 128						*/
/*		#define s 63						*/
/*		#define mbits 23					*/
/************************************************************************/

/***********************************************************************/
/***********************************************************************/
/*Initialization routines.                                             */
/***********************************************************************/
/***********************************************************************/
void initiran ( int seqnum, int gseed, int state[2][2][L] )
{
/*
  ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
  c  This routine initializes the state for the seqnum(th) member     
  c  of the family of pseudorandom sequences.  initiran must          
  c  be called once (and only once) for each sequence from which      
  c  random binary or integer values are wanted.                      
  c                                                                   
  c  L, k, s, and mbits are properties associated with the family     
  c  of generators used by this package.  L and k represent a         
  c  particular primitive trinomial (mod 2), s is the "canonical bit" 
  c  associated with that trinomial, and mbits is the number of       
  c  mantissa bits used by the floating point pseudorandom generator. 
  c  for L = 158, state must have a declared dimension of at least    
  c  4 x 158 = 632 in the calling program.                            
  c                                                                   
  c  After initializing a sequence with initiran, multiple calls      
  c  can be made to irand or brand64,  and the state vector will      
  c  stay current.   The state vector must not be modified by the user
  c  code.   If more than one sequence is to be active at any time,   
  c  there must be a searate state vector for each active sequence.   
  c                                                                   
  c  gseed is the integer value of the user-supplied "global seed."   
  c  The global seed is intended to be fixed for the duration of the  
  c  run and should be the same value for every sequence on every    
  c  processor.   It can be used to modify the random data, in a      
  c  predictable way, on a per-run basis.  seqnum must be in the      
  c  range {0,1,2,3,..., 65535 = 2^16 - 1 = z'0000ffff'} and gseed    
  c  must be in the range {0,1,2,3,..., 65534 = 2^16 - 2 = z'0000fffe'}.
  ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
*/

   int a, q, r, t31m1, seed, i, hi, lo;
   int treg1, treg2, highbit1, highbit2;

   a = 16807;  q = 127773;  r = 2836;
   t31m1 = 0x7fffffff;

   tausinit(seqnum, gseed, &treg1, &treg2);

   /*Initializing the first state register*/
   seed = treg1 & t31m1;
   highbit1 = seed ^ treg1;

   state[0][0][L-1] = 0;

   for (i = 0; i < L-1; i++)
   {
      hi = seed/q;
      lo = seed - (hi*q);
      seed = a*lo - r*hi;

      if ( seed <= 0 )
         seed = seed + t31m1;

      state[0][0][i] = seed + seed;
   }

   /* Incorporate the high bit of the taus. reg. in the  */
   /* fibonacci register, in order to maintain uniqueness*/
   if ( highbit1 == 0 )
      state[0][0][L-2] = state[0][0][L-2] & t31m1;
   else
      state[0][0][L-2] = state[0][0][L-2] | highbit1;

   state[0][0][s] = state[0][0][s] | 1;

 
   /*Initializing the second state register*/
   seed = (treg2 & t31m1);
   highbit2 = (seed ^ treg2);

   state[0][1][L-1] = 0;

   for (i = 0; i < L-1; i++)
   {
     hi = seed/q;
     lo = seed - (hi*q);
     seed = a*lo - r*hi;
     if ( seed <= 0 )
        seed += t31m1;
     state[0][1][i] = seed + seed;
   }

   /* Incorporate the high bit of the taus. reg. in the  */
   /* fibonacci register, in order to maintain uniqueness*/
   if ( highbit2 == 0 )
      state[0][1][L-2] = state[0][1][L-2] & t31m1;
   else
      state[0][1][L-2] = state[0][1][L-2] | highbit2;

   state[0][1][s] = state[0][1][s] | 1;

   return;
}


void initfran ( int seqnum, int gseed, double state[2][L] )
{
/*
  cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
  c  This routine initializes the state for the seqnum(th) member     
  c  of the family of pseudorandom sequences.  initfran must          
  c  be called once (and only once) for each sequence from which      
  c  random floating point values are wanted.                         
  c                                                                   
  c  l, k, s, and mbits are properties associated with the family     
  c  of generators used by this package.  L and k represent a         
  c  particular primitive trinomial (mod 2), s is the "canonical bit" 
  c  associated with that trinomial, and mbits is the number of       
  c  mantissa bits used by the floating point pseudorandom generator. 
  c  for L = 158, state must have a declared dimension of at least    
  c  2 x 158 = 316 in the calling program.                            
  c                                                                   
  c  After initializing a sequence with initfran, multiple calls      
  c  can be made to frand,  and the state vector will stay current.   
  c  The state vector must not be modified by the user code.   If     
  c  more than one sequence is to be active at any time, there must   
  c  be a searate state vector for each active sequence.              
  c                                                                   
  c  gseed is the integer value of the user-supplied "global seed."   
  c  The global seed is intended to be fixed for the duration of the  
  c  run and should be the same value for every sequence on every     
  c  processor.   It can be used to modify the random data, in a      
  c  predictable way, on a per-run basis.  seqnum must be in the      
  c  range {0,1,2,3,..., 65535 = 2^16 - 1 = z'0000ffff'} and gseed    
  c  must be in the range {0,1,2,3,..., 65534 = 2^16 - 2 = z'0000fffe'}.
  ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
*/

   int hi, lo, a, q, r, seed, i, mask;
   int t31m1, t30m1, treg, tregx, highbit;
   double scale;

   a = 16807;  q = 127773;  r = 2836;
   t31m1 = 0x7fffffff;
   t30m1 = 0x3fffffff;

   tausinit(seqnum, gseed, &treg, &tregx);
   scale = 1.0;

   for (i = 0; i < mbits; i++)
      scale = scale *0.5;

   mask = (1 << mbits) - 2;
   seed = (treg & t31m1);
   highbit = (seed ^ treg);

   /*Initialize register in state vector*/
   state[0][0] = scale * ((float)(seqnum + seqnum));
   state[0][L-1] = 0.0;

   for (i = 1; i < L-2; i++)
   {
      hi = seed/q;
      lo = seed -(hi*q);
      seed = (a*lo) - (r*hi);
      
      if(seed <= 0)
         seed = seed + t31m1;

      state[0][i] = scale * ( (float) ((seed >> (31 - mbits)) & mask) );
   }

   hi = seed/q;
   lo = seed - (hi*q);
   seed = (a*lo) - (r*hi);

   if( seed <= 0 )
      seed = seed + t31m1;

   if ( highbit == 0 )
      seed = ( seed & t30m1 );
   else
      seed = ( seed | ( highbit >> 1 ) );

   state[0][L-2] = scale * ( (float) ((seed >> (31-mbits)) & mask) );
   state[0][s] = state[0][s] + scale;

   return;
}
/***********************************************************************/
/***********************************************************************/
/*End of initialization routines.                                      */
/***********************************************************************/
/***********************************************************************/






/***********************************************************************/
/***********************************************************************/
/*Generation routines.                                                 */
/***********************************************************************/
/***********************************************************************/
void irand ( int64 n, int state[2][2][L], int x[] )
{
/*
  ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
  c This routine returns n random 32-bit integers in the array x.
  c                                                                   
  c The state vector is updated to continue producing random numbers 
  c from the current sequence by subsequent calls to this routine.   
  c The state vector should not be modified by the calling routine,  
  c unless no more values are required from the current sequence.    
  cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
*/

   int t31m1, t32m2;
   int kk, i, old, new;
   int64 xptr, temp, ii;

   t31m1 = 0x7fffffff;
   t32m2 = 0xfffffffe;
   old = 1; new = 0; xptr = 0;


   while ( xptr < n )
   {
      old = new;
      new = 1 - new;
      /*k is defined in the header file to be 128*/
      kk = min(k, (n-xptr));

      for (i = 0; i < kk; i++)
      {
         state[new][0][i] = state[old][0][k-kk+i] + state[old][0][L-kk+i];
         state[new][1][i] = state[old][1][k-kk+i] + state[old][1][L-kk+i];
      }

      for (i = kk; i < L; i++)
      {
         state[new][0][i] = state[old][0][i-kk];
         state[new][1][i] = state[old][1][i-kk];
      }

      temp = min(xptr+kk, n);
      for (ii = xptr; ii < temp; ii++)
      {
         x[ii] = ( state[new][0][kk-(ii-xptr)-1] >> 1 ) & t31m1;
         x[ii] = ( state[new][1][kk-(ii-xptr)-1] & t32m2 ) ^ x[ii];
      }

      xptr = xptr + kk;
   }

   /*Update "old" state for subsequent calls*/
   if ( new == 1 )
   {
      for (i = 0; i < L; i++)
      {
         state[old][0][i] = state[new][0][i];
         state[old][1][i] = state[new][1][i];
      }
   }

   return;
}



void frand ( int64 n, double state[2][L], double x[] )
{
/*
  ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
  c This routine returns n random real numbers in the array, x.      
  c                                                                   
  c The state vector is updated to continue producing random numbers 
  c from the current sequence by subsequent calls to this routine.   
  c The state vector should not be modified by the calling routine,   
  c unless no more values are required from the current sequence.    
  cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
*/

   int kk, i, old, new;
   int64 xptr, temp, ii;
   old = 1; new = 0; xptr = 0;

   while ( xptr < n )
   {
      old = new;
      new = ( new ^ 1 );
      kk = min((n-xptr), k);

      for (i = 0; i < kk; i++)
      {
         state[new][i] = state[old][(k-kk+i)] + state[old][(L-kk+i)];
      }

      for (i = 0; i < kk; i++)
      {
         if ( state[new][i] >= 1.0 )
            state[new][i] = state[new][i] - 1.0;
      }

      for (i = kk; i < L; i++)
      {
         state[new][i] = state[old][i-kk];
      }

      temp = min((xptr + kk), n);

      for (ii = xptr; ii < temp; ii++)
      {
         x[ii] = state[new][(kk - (ii+1-xptr))];
      }

      xptr = xptr + kk;
   }

   /*Update 'old' state for subsequent calls*/

   if ( new == 1 )
   {
      for (i = 0; i < L; i ++)
      {
         state[old][i] = state[new][i];
      }
   }

   return;
}


void brand64 ( int64 n, int state[2][2][L], uint64 x[] )
{
/*
 cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
 c Used for producing random bit streams on machines with 64-bit integers,
 c this routine returns n words of random 64-bit integers in the array x.
 c Brand64 calls irand to generate 32-bit integers, which it pastes
 c together as 64-bit integers.  The result is the same string of bits
 c that would be produced on a 32-bit machine by calling irand with a
 c (2 x n)-long x vector.  Thus, for creating binary streams, use irand
 c on 32-bit machines, and brand64 on 64-bit machines.
 c
 c The state vector is updated to continue producing random numbers
 c from the current sequence by subsequent calls to this routine.
 c The state vector should not be modified by the calling routine,
 c unless no more values are required from the current sequence.
 ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
*/
   uint64 low32 = 0x00000000ffffffff;
   int i, j;
   int64 xptr, nleft, ii;

   const int wrklen = 8192;

   int  wrk[8192];
   uint64 wrk8[8192];

   xptr = 1;
   nleft = n;

   while ( nleft > 0 )
   { 
      if( (2*nleft) > wrklen )
      {
         /* generate twice as many 32-bit ints */
         irand ( (int64)wrklen, state, wrk );

         /* copy to array of uint64 and prevent sign ext 32->64 bits */
         for (i = 0; i < wrklen; i++)
         {
            wrk8[i] = wrk[i] & low32;
         }

         /* paste pairs of ints to make uint64s */
         for ( ii = xptr-1, j = 0;  ii < xptr+(wrklen/2)-1;  ii++, j++ )
         {
            x[ii] = wrk8[2*j] | (wrk8[2*j+1] << 32);
         }

         xptr = xptr + wrklen/2;
         nleft = nleft - wrklen/2;
      }

      else
      {
         /* generate twice as many 32-bit ints */
         irand ( 2*nleft, state, wrk );

         /* copy to array of uint64 and prevent sign ext 32->64 bits */
         for (i = 0; i < 2*nleft; i++)
         {
            wrk8[i] = wrk[i] & low32;
         }

         /* paste pairs of ints to make uint64s */
         for ( ii = xptr-1, j = 0;  ii < xptr+nleft-1;  ii++, j++ )
         {
            x[ii] = wrk8[2*j] | (wrk8[2*j+1] << 32);
         }

         xptr = xptr + nleft;
         nleft = 0;
      }
   }

   return;
}
/***********************************************************************/
/***********************************************************************/
/*End of generation routines.                                          */
/***********************************************************************/
/***********************************************************************/





/***********************************************************************/
/***********************************************************************/
/*Bookkeeping routines                                                 */
/***********************************************************************/
/***********************************************************************/

/*nseqs must be greater than or equal to npes to avoid division by zero*/
void get_penum ( int seqnum, int npes, int nseqs, int *penum )
{
  /*
   cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
   c given  a sequence number (seqnum), number of pes (npes), and 
   c a number of sequences (nseqs) this routine determines the 
   c number of the pe on which sequence number seqnum is located.            
   c Assumes that penum is in the range {0,1,2,...,npes-1} and that   
   c sequences are numbered starting from 0.                          
   cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
  */

   int q, r;

   q = nseqs/npes;
   r = nseqs - (npes*q);

   (*penum) = seqnum / (q+1);

   if ( seqnum >= ((q+1)*r) )
      (*penum) = r + ((seqnum - ((q+1)*r))/q);

   return;
}



void get_seq_num ( int penum, int npes, int nseqs,
		int *seqfirst, int *seqlast )
{
  /*
   cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
   c This routine computes the indices of the first and last 
   c sequences that will be associated with pe number penum, when 
   c there are npes, and nseqs sequences.  Assumes that penum is    
   c in the range{0,1,2,...,npes-1} and that sequences are numbered
   c starting from 0.
   cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
  */

   int q, r;

   q = nseqs/npes;
   r = nseqs - (npes*q);

   (*seqfirst) = (penum*q) + min(penum, r);

   (*seqlast) = (*seqfirst) + q - 1;

   if ( penum < r )
      (*seqlast) = (*seqlast) +1;

   return;
}


void get_seq_bounds ( int seqnum, int nseqs, int64 lendata, int chnksize,
			int64 *datafirst, int64 *datalast)
{
  /*
   ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
   c Given a particular sequence number (seqnum), the number of       
   c sequences (nseqs), and the total length of the data (lendata),  
   c this will compute datafirst and datalast, the positions in the   
   c overall data set that sequence number seqnum occupies.   This    
   c routine will force each sequence to have an integer number of 
   c data values of length chnksize (with the exception of the last        
   c sequence, which will have any partial chunk left over from a 
   c total data length of lendata).  Assumes that data arrays are
   c numbered starting from 1, and that sequences are numbered  
   c statring from 0. 
   ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
  */

   int64 nchunks, q, r;

   nchunks = lendata/chnksize;
   q = nchunks/nseqs;
   r = nchunks - (nseqs*q);

   (*datafirst) = chnksize*(seqnum*q + min(seqnum, r)) + 1;

   (*datalast) = (*datafirst) + q*chnksize - 1;

   if ( seqnum < r )
      (*datalast) = (*datalast) + chnksize;

   if ( seqnum == (nseqs - 1) )
      (*datalast) = lendata;

   return;
}
/***********************************************************************/
/***********************************************************************/
/*End of Bookkeeping routines.                                         */
/***********************************************************************/
/***********************************************************************/






/***********************************************************************/
/***********************************************************************/
/*User interfaces for the random number generator.                     */
/***********************************************************************/
/***********************************************************************/
void get_my_idata ( int penum, int npes, int nseqs, int64 lendata,
			int chnksize, int gseed, int x[],
			int64 *datafirst, int64 *datalast )
{
  /*
   cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
   c returns the integer data belonging to the PE numbered penum,     
   c given that there are npes PEs and given that the user has        
   c specified nseqs sequences, a total data length of lendata        
   c integers, a chunk size of chnksize integers, and a global seed   
   c value of gseed.  The data belonging to PE penum will be returned 
   c in the integer array, x, starting with x(1),  but will be used  
   c by the calling program as data items datafirst through datalast 
   c in the overall lendata-long set that resides across all PEs.  
   c For this routine to work correctly, npes must be less than the  
   c total number of chunks in the data set.  I.e. npes must be less
   c than lendata/chnksize.  The size of x must be declared to be at 
   c least as large as the value of maxlen returned by the routine 
   c check_maxlen.  Of course, this will be about lendata/npes, but 
   c the value of maxlen returned by check_maxlen is the exact value 
   c needed by the worst-case PE.        
   ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
  */

   int seq_frst;
   int64 mydata_len, dlen, dlast, dfrst, nskip, mydata_left, ix;
   int64 nchnks, q, r, ch_frst, ch_last;
   int64 nseq_len1, len_data1, seq_frst2, datafirst2;
   int64 lendatac, seq_len1, seq_len2, seq_len1c, seq_len2c, rc;

   int state[2][2][L];

   nchnks = lendata/chnksize;
   q = nchnks/npes;
   r = nchnks - (npes*q);

   ch_frst = penum*q + min(penum, r) + 1;
   ch_last = (penum + 1)*q + min((penum+1), r);

   (*datafirst) = chnksize*(ch_frst - 1) + 1;
   (*datalast)  = chnksize*(ch_last - 1) + chnksize;

   if ( penum == (npes-1) )
      (*datalast) = lendata;

   mydata_len = (*datalast) - (*datafirst) +1;

   lendatac = lendata/chnksize;
   seq_len2c = lendatac/nseqs;
   seq_len1c = seq_len2c + 1;
   rc = lendatac - (seq_len2c*nseqs);

   seq_len2 = chnksize * seq_len2c;
   seq_len1 = chnksize * seq_len1c;
   nseq_len1 = rc;

   len_data1 = nseq_len1*seq_len1;

   if ( (*datafirst) > len_data1 )
   {
      datafirst2 = (*datafirst) - len_data1;
      seq_frst2 = datafirst2/seq_len2;
      nskip = datafirst2 - (seq_frst2*seq_len2) - 1;
      seq_frst = seq_frst2 + nseq_len1;
   }
   else
   {
      seq_frst = (*datafirst)/seq_len1;
      nskip = (*datafirst) - (seq_frst*seq_len1) - 1;
   }

   get_seq_bounds(seq_frst, nseqs, lendata, chnksize, &dfrst, &dlast);

   dlen = dlast - dfrst + 1;

   initiran(seq_frst, gseed, state);
   irand(nskip, state, x);

   dlen = dlen - nskip;
   mydata_left = mydata_len;
   ix = 0;

   irand(dlen, state, &x[ix]);

   mydata_left = mydata_left - dlen;
   ix = ix + dlen;

   while ( mydata_left > 0 )
   {
      seq_frst = seq_frst + 1;
      get_seq_bounds(seq_frst, nseqs, lendata, chnksize, &dfrst, &dlast);

      dlen = min((dlast-dfrst+1), mydata_left);

      initiran(seq_frst, gseed, state);
      irand(dlen, state, &x[ix]);
    
      mydata_left = mydata_left - dlen;
      ix = ix + dlen;
   }

   return;
}


void get_my_fdata ( int penum, int npes, int nseqs, int64 lendata,
			int chnksize, int gseed, double x[],
			int64 *datafirst, int64 *datalast )
{
  /*
   cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
   c returns the floating point data belonging to the PE numbered
   c penum, given that there are npes PEs and given that the user
   c has specified nseqs sequences, a total data length of lendata        
   c integers, a chunk size of chnksize reals, and a global seed      
   c value of gseed.  The data belonging to PE penum will be returned
   c in the real array, x, starting with x(1),  but will be used by
   c the calling program as data items datafirst through datalast 
   c in the overall lendata-long set that resides across all PEs.
   c For this routine to work correctly, npes must be less than the
   c total number of chunks in the data set.  I.e. npes must be less
   c than lendata/chnksize.  The size of x must be declared to be at
   c least as large as the value of maxlen returned by the routine
   c check_maxlen.  Of course, this will be about lendata/npes, but 
   c the value of maxlen returned by check_maxlen is the exact value
   c needed by the worst-case PE.              
   cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
  */

   int seq_frst;
   int64 mydata_len, dlen, dlast, dfrst, nskip, mydata_left, ix;
   int64 nchnks, q, r, ch_frst, ch_last;
   int64 nseq_len1, len_data1, seq_frst2, datafirst2;
   int64 lendatac, seq_len1, seq_len2, seq_len1c, seq_len2c, rc;

   double state[2][L];

   nchnks = lendata/chnksize;
   q = nchnks/npes;
   r = nchnks - (npes*q);

   ch_frst = penum*q + min(penum, r) + 1;
   ch_last = (penum + 1)*q + min((penum+1), r);

   (*datafirst) = chnksize*(ch_frst - 1) + 1;
   (*datalast)  = chnksize*(ch_last - 1) + chnksize;

   if ( penum == (npes - 1) )
      (*datalast) = lendata;

   mydata_len = (*datalast) - (*datafirst) + 1;

   lendatac = lendata/chnksize;
   seq_len2c = lendatac/nseqs;
   seq_len1c = seq_len2c + 1;
   rc = lendatac - (seq_len2c*nseqs);

   seq_len2 = chnksize * seq_len2c;
   seq_len1 = chnksize * seq_len1c;
   nseq_len1 = rc;

   len_data1 = nseq_len1*seq_len1;

   if ( (*datafirst) > len_data1 )
   {
      datafirst2 = (*datafirst) - len_data1;
      seq_frst2 = datafirst2/seq_len2;
      nskip = datafirst2 - (seq_frst2*seq_len2) - 1;
      seq_frst = seq_frst2 + nseq_len1;
   }
   else
   {
      seq_frst = (*datafirst)/seq_len1;
      nskip = (*datafirst) - (seq_frst*seq_len1) - 1;
   }

   get_seq_bounds(seq_frst, nseqs, lendata, chnksize, &dfrst, &dlast);

   dlen = dlast - dfrst + 1;

   initfran(seq_frst, gseed, state);
   frand(nskip, state, x);

   dlen = dlen - nskip;
   mydata_left = mydata_len;
   ix = 0;

   frand(dlen, state, &x[ix]);

   mydata_left = mydata_left - dlen;
   ix = ix + dlen;

   while ( mydata_left > 0 )
   {
      seq_frst = seq_frst + 1;
      get_seq_bounds(seq_frst, nseqs, lendata, chnksize, &dfrst, &dlast);

      dlen = min((dlast-dfrst+1), mydata_left);

      initfran(seq_frst, gseed, state);
      frand(dlen, state, &x[ix]);
    
      mydata_left = mydata_left - dlen;
      ix = ix + dlen;
   }

   return;
}



void get_my_bdata64 ( int penum, int npes, int nseqs, int64 lendata,
			int chnksize, int gseed, uint64 x[],
			int64 *datafirst, int64 *datalast )
{
  /*
   ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
   c returns the 64-bit integer data belonging to the PE numbered
   c penum, given that there are npes PEs and given that the user
   c has specified nseqs sequences, a total data length of lendata
   c 64-bit integers, a chunk size of chnksize 64-bit integers,  
   c and a global seed value of gseed.  The data belonging to PE
   c penum will be returned in the 64-bit integer array, x, 
   c starting eith x(1) but  will be used by the calling program
   c as data items datafirst through datalast in the overall 
   c lendata-long set that resides across all PEs.  For this
   c routine to work correctly, npes must be less than the total 
   c number of chunks in the data set.  I.E. npes must be less
   c than lendata/chnksize.  The size of x must be declared to be
   c at least as large as the value of maxlen returned by the 
   c routine check_maxlen.  Of course, this will be about lendata/
   c npes, but the value of maxlen returned by check_maxlen is the
   c exact value needed by the the worst-case PE.                 
   cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
  */

   int seq_frst;
   int64 mydata_len, dlen, dlast, dfrst, nskip, mydata_left, ix;
   int64 nchnks, q, r, ch_frst, ch_last;
   int64 nseq_len1, len_data1, seq_frst2, datafirst2;
   int64 lendatac, seq_len1, seq_len2, seq_len1c, seq_len2c, rc;

   int state[2][2][L];

   nchnks = lendata/chnksize;
   q = nchnks/npes;
   r = nchnks - (npes*q);

   ch_frst = penum*q     + min(penum, r) + 1;
   ch_last = (penum+1)*q + min((penum+1), r);

   (*datafirst) = chnksize*(ch_frst - 1) + 1;
   (*datalast)  = chnksize*(ch_last - 1) + chnksize;

   if ( penum == (npes - 1) )
      (*datalast) = lendata;

   mydata_len = (*datalast) - (*datafirst) + 1;

   lendatac = lendata/chnksize;
   seq_len2c = lendatac/nseqs;
   seq_len1c = seq_len2c + 1;
   rc = lendatac - (seq_len2c*nseqs);

   seq_len2 = chnksize * seq_len2c;
   seq_len1 = chnksize * seq_len1c;
   nseq_len1 = rc;

   len_data1 = nseq_len1*seq_len1;

   if ( (*datafirst) > len_data1 )
   {
      datafirst2 = (*datafirst) - len_data1;
      seq_frst2 = datafirst2/seq_len2;
      nskip = datafirst2 - (seq_frst2*seq_len2) - 1;
      seq_frst = seq_frst2 + nseq_len1;
   }
   else
   {
      seq_frst = (*datafirst)/seq_len1;
      nskip = (*datafirst) - (seq_frst*seq_len1) - 1;
   }

   get_seq_bounds(seq_frst, nseqs, lendata, chnksize, &dfrst, &dlast);

   dlen = dlast - dfrst + 1;

   initiran(seq_frst, gseed, state);
   brand64(nskip, state, x);

   dlen = dlen - nskip;
   mydata_left = mydata_len;
   ix = 0;

   brand64(dlen, state, &x[ix]);

   mydata_left = mydata_left - dlen;
   ix = ix + dlen;

   while ( mydata_left > 0 )
   {
      seq_frst = seq_frst + 1;
      get_seq_bounds(seq_frst, nseqs, lendata, chnksize, &dfrst, &dlast);

      dlen = min((dlast-dfrst+1), mydata_left);

      initiran(seq_frst, gseed, state);
      brand64(dlen, state, &x[ix]);
    
      mydata_left = mydata_left - dlen;
      ix = ix + dlen;
   }

   return;
}
/***********************************************************************/
/***********************************************************************/
/*End of user interfaces.                                              */
/***********************************************************************/
/***********************************************************************/





void check_maxlen ( int npes, int nseqs, int64 lendata,
			int chnksize, int64 *maxlen )
{
  /*
   cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
   c exact value of maxlen needed by the worst case PE
   cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
  */

   int64 penum, nchnks, q, r;
   int64 ch_frst, ch_last, datafirst, datalast, mydata_len;

   penum = 0;
   nchnks = lendata/chnksize;
   q = nchnks/npes;
   r = nchnks - npes*q;

   ch_frst = penum*q + min(penum, r) + 1;
   ch_last = (penum+1)*q + min((penum+1), r);

   datafirst = chnksize*(ch_frst - 1) + 1;
   datalast = chnksize*ch_last;

   if ( npes == 1 )
      datalast = lendata;

   mydata_len = datalast - datafirst + 1;
   (*maxlen) = mydata_len;


   penum = npes - 1;
   nchnks = lendata/chnksize;
   q = nchnks/npes;
   r = nchnks - npes*q;

   ch_frst = penum*q + min(penum, r) + 1;
   ch_last = (penum+1)*q + min((penum+1), r);

   datafirst = chnksize*(ch_frst - 1) + 1;
   datalast = lendata;

   mydata_len = datalast - datafirst + 1;

   if ( (*maxlen) < mydata_len )
      (*maxlen) = mydata_len;


   return;
}


void tausinit ( int seqnum, int gseed, int* treg64, int* treg96 )
{
  /*
   cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
   c  uses a 32-bit tausworthe generator to map the pair (seqnum,gseed)   c
   c  into a 32-bit pseudorandom number, for subsequent use in filling    c
   c  out the fibonacci register.  This routine produces two values,      c
   c  treg64 and treg96, which represent 64 and 96 steps of the           c
   c  initial fill.  Since the calling routine will only be using         c
   c  the low 31 bits (mod 2^31 - 1), there are 6 cases to avoid.         c
   c  The bad values for the tausworthe register are (in order):          c
   c    T^(-64)(80000000),T^(-64)(7fffffff),T^(-64)(ffffffff),            c
   c    T^(-96)(80000000),T^(-96)(7fffffff),T^(-96)(ffffffff).            c
   c  these cases correspond to (gseed,seqnum) pairs (in decimal):        c
   c  (43553,65535), (26141,65535), (52283,65535),                        c
   c  (50425,37887), (17320,35839), (34642, 6143).                        c
   c  these are mapped to multiples of the prime number 9349, which       c
   c  is approximately 65536/7 and which do not correspond to valid       c
   c  initial values of seqnum and gseed that would be passed into        c
   c  this routine.                                                       c
   cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
  */
     
   int gshft, i;
   int seqmask, treg;
   int tbad[6] = { 0xAA220000, 0x661E0000, 0xCC3C0000,
		   0xC4FA9400, 0x43A98C00, 0x87531800 };   

   int t_ok[6] = { 0x00002485, 0x0000490A, 0x00006D8F,
		   0x00009214, 0x0000B699, 0x0000DB1E };
   gshft = 16;

   seqmask = 0x0000ffff;
/* this should be: (gseed & seqmask) << gshft; */
   treg = (int)(gseed + 1) << gshft;
   treg = treg | ((seqnum + 1) & seqmask);
   for (i = 0; i < 6; i++)
   {
      if (treg == tbad[i])
         treg = t_ok[i];
   }

   taus32 ( &treg, 64 );
   *treg64 = treg;
   taus32 ( &treg, 32 );
   *treg96 = treg;
  
   return;
}



void taus32 ( int* treg, int nsteps )
{
  /*
   cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
   c  steps the 32-bit tausworthe generator nsteps times, using the       c
   c  mod 2 primitive polynomial (32,7,5,3,2,1,0).  The table entries     c
   c  are used to look up the parity of the least significant 7 bits.     c
   c  Tausworthe generators are discussed in Numerical Recipes by         c
   c  Press, et al.  Cambridge University Press, c1986, pp 209-213.       c
   cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
  */

   int i, bit31, low7;

   int table0[128] = {
	0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1, 1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,
	0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1, 1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,
	1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0, 0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,
	1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0, 0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1};

   int table1[128] = {
	1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0, 0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,
	1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0, 0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,
	0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1, 1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,
	0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1, 1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0};

   int t31   = 0x80000000;
   int t32m1 = 0xFFFFFFFF;

   for (i = 0; i < nsteps; i++)
   {
      bit31 = ((*treg) & t31);
      low7  = ((*treg) & 127);
      (*treg)  = ((*treg) << 1) & t32m1;
      if(bit31 == 0)
         (*treg) = (*treg) | table0[low7];
      else
         (*treg) = (*treg) | table1[low7];
   }

   return;
}


/********************************************************************/
/***********************END OF FILE: prandom.c********************* */
/********************************************************************/
