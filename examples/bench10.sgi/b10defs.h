#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef CRAY
#include <mpp/shmem.h>
#else
#include <shmem.h>
#endif

#ifdef CRAY
#define my_pe()           shmem_my_pe()
#define num_pes()         shmem_n_pes()
/* #define shmem_init() */
#define barrier()         shmem_barrier_all()
#endif

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

