/************************************************************************/
/*File: prandom.h							*/
/*Date created:  June 14, 1999						*/
/*Current Date:  June 23, 1999						*/
/*Description: Prototypes for functions defined in the file, prandom.c	*/
/************************************************************************/


#ifndef PRANDOM_H
#define PRANDOM_H


/*Definitions of constants used by functions in prandom.c  If conditions */
/*change, altering the value of the constants here will affect all       */
/*associated functions in the file prandom.c.   In order to match the    */
/*amount of precision in the FORTRAN generated numbers, the state and    */
/*output arrays for floating point numbers need to be declared as doubles*/
/*Also, for all datatypes, the user's nseqs must be greater than npes in */
/*order to avoid dividing by zero in the bookkeeping routine get_penum.  */
/*Furthermore, npes*chnksize <= lendata and penum must be between zero   */
/*and (npes-1).*/

#define L 158                                          
#define k 128                                          
#define s 63                                           
#define mbits 23                                       
#define min(A, B) ((A) < (B) ? (A) : (B))
#define int64	long
#define uint64	unsigned long


/*************************INITIALIZATION FUNCITONS***********************/

void initiran ( int seqnum, int gseed, int state[2][2][L] );

void initfran ( int seqnum, int gseed, double state[2][L] );


/***************************GENERATION FUNCTIONS*************************/

void irand  ( int64 n, int state[2][2][L], int x[] );

void frand  ( int64 n, double state[2][L], double x[] );

void brand64( int64 n, int state[2][2][L], uint64 x[] );


/***************************BOOKKEEPING FUNCTIONS************************/

/*In get_penum, must have nseqs > npes to avoid dividing by zero.*/
void get_penum ( int seqnum, int npes, int nseqs, int *penum );

void get_seq_num ( int penum, int npes, int nseqs,
			int *seqfirst, int *seqlast );

void get_seq_bounds ( int seqnum, int nseqs, int64 lendata, int chnksize, 
			int64 *datafirst, int64 *datalast );


/*****************************USER'S INTERFACE***************************/

void get_my_idata ( int penum, int npes, int nseqs, int64 lendata,
	int chnksize, int gseed, int x[], int64*datafirst, int64*datalast );

void get_my_fdata ( int penum, int npes, int nseqs, int64 lendata,
	int chnksize, int gseed, double x[], int64*datafirst, int64*datalast );

void get_my_bdata64(int penum, int npes, int nseqs, int64 lendata,
	int chnksize, int gseed, uint64 x[], int64*datafirst, int64*datalast );


/******************************OTHER FUNCTIONS***************************/

void check_maxlen ( int npes, int nseqs, int64 lendata,
			int chnksize, int64*maxlen );

void tausinit ( int seqnum, int gseed, int*treg64, int*treg96 );

void taus32 ( int*treg, int steps );


#endif
/************************************************************************/
/*************************END OF FILE: prandom.h*************************/
/************************************************************************/
