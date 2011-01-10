/* bench6.h */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include <mpp/shmem.h>
#include <mpi.h>

# include <sys/types.h>
/* already handled above
 * # include <shmem.h>
 */
#define int64		long
#define uint64		unsigned long
#define MY_INT64	MPI_LONG

#ifndef SW_POPCNT
# define popcnt  _popcnt
# include <machine/builtins.h>
#endif

#define MAXPES		64

#define WS		64
#define LOGWS		6
#define WSMASK		63

/* macro to do RMASK n - n 1's at low end */
#define RMASK(N)	( ((uint64)1 << (N)) - 1 )

#define min(A, B)	( (A) < (B) ? (A) : (B) )

/*-------------------------------------------------------------------------
*  A bunch of constants for this benchmark
*--------------------------------------------------------------------------*/
#define MAXLOOPS	400
#define DEFLOOPS	100

/* Maximum length of S stream */
#define SLENMAX		10000000

/*  Array size (# of equations to solve) */
#define MSIZE0		700

/*  Seed for random number generator */
#define ISEED0		99906

/* The number of bits (including don't care bits) in PAT */
#define PATLEN		22

/* The default pattern to be found (with x's in the don't care positions) */
/* const char PAT[23]  = "000xxx100xx110x10x1111"; */

/* Maximum number of words in input bitstream */
#define SNWMAX		(SLENMAX/WS + 1)

/* Number of slots for matches with allowance for possible non-random data */
#define MAXMATCH0	700

/* Number of words required to store MSIZE+1 columns of bits */
#define EQNW0		(MSIZE0/WS + 1)

/* Block size for speeding up Gaussian reduction */
#define BS		6

/* Biggest 2^bs for block reduction, with bs=8 */
#define TWOB		256

