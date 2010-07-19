/*-------------------------------------------------------------------------
*  S6 generates a random stream  s  of slen bits packed in snw words
*
*  It initializes the  startbits  array to (slen-plen+1) 1's
-------------------------------------------------------------------------*/

#include "bench6.h"

static char cvs_info[] = "BMARKGRP $Date: 2005/01/10 21:17:26 $ $Revision: 1.2 $ $RCSfile: s6.c,v $ $Name:  $";


s6 (	uint64* s,
	uint64* startbits,
	int slen, int snw, int plen, int iseed, int loop )
{
   int wsmask = WSMASK;
   int sbits, stbits, i;
   int64 snw8 = snw, myf=0, myl=0;

   /* fill snw words of s with random bits */
   /* the arg 'loop' gets a distinct random stream for each loop */

   get_my_bdata64 ( loop, 1, 1, snw8, 1, -iseed, s, &myf, &myl );

   /* erase so just slen bits */
   /* if sbits=0, snw full words for s */
   /* if sbits>0, keep just sbits bits of s[snw-1] */

   sbits = slen & wsmask;
   if (sbits)
      s[snw-1] &= RMASK(sbits);

   /* initialize startbits to all 1's */

   for ( i=0; i<snw; ++i )
      startbits[i] = (uint64)(-1);

   /* get rid of all bits after (slen-plen+1) */

   if ( sbits == 0 )
      sbits = 64;
   stbits = sbits - plen + 1;
   if ( stbits >= 0 )
      startbits[snw-1] = RMASK(stbits);
   else
   {
      startbits[snw-1] = 0;
      startbits[snw-2] &= RMASK(64 + stbits);
   }

#ifdef PrintBits
   if ( slen < 10000 )
   {
      printf("3210987654321098765432109876543210987654321098765432109876543210\n
\n");
#ifdef CRAY
      for ( i=0; i<snw; ++i )
         printf( "%064B\n", s[i] );
      printf("\n");
      for ( i=0; i<snw; ++i )
         printf( "%064B\n", startbits[i] );
      printf("\n");
#else
      int j;  uint64 word;
      for ( i=0; i<snw; ++i )
      {
         word = s[i];
         for ( j = 63; j >= 0; j-- )
            printf( "%d", (int) ( (word >> j) & (uint64)1 ) );
         printf("\n");
      }
      printf("\n");
      for ( i=0; i<snw; ++i )
      {
         word = startbits[i];
         for ( j = 63; j >= 0; j-- )
            printf( "%d", (int) ( (word >> j) & (uint64)1 ) );
         printf("\n");
      }
      printf("\n");
#endif
   }
#endif
}
