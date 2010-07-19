#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <shmem.h>

/* #define INTRINSICS   Pull in popcnt intrinsic */
#define SW_POPCNT	1   /* Pull in subroutine substitution for popcnt */

/* Maximum number of solutions, a little larger than needed for N = 16 */
#define MAXSOL       15000000

#define DFLTSIZE     16
#define MAXSIZE      DFLTSIZE
#define MINSIZE      4
#define DFLTWIDTH    128
#define MAXWIDTH     1024
#define DFLTSLVL     3
#define MAXSLVL      5
#define MINSLVL      2

/* Number of bits per word.  This code assumes this is 64. */
#define BITSIZ       64

