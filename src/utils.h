#ifndef _UTILS_H
#define _UTILS_H 1

#ifdef DEBUG

#include <stdio.h>
#include <stdlib.h>

#include "state.h"

#define IF_DEBUGGING(x) do { x ; } while (0)

#else /* ! DEBUG */

#define IF_DEBUGGING(x)

#endif /* DEBUG */

/*
 * if we haven't initialized through start_pes() then try to do
 * something constructive.  Obviously can't use __shmem_trace()
 * because nothing has been initialized.
 *
 */

#define INIT_CHECK()							\
  IF_DEBUGGING(								\
	       if (GET_STATE(pe_status) != PE_RUNNING) {		\
		 fprintf(stderr,					\
			 "Error: OpenSHMEM library has not been initialized\n" \
			 );						\
		 exit(1);						\
		 /* NOT REACHED */					\
	       }							\
	       )

/*
 * make sure a target PE is within the assigned range
 *
 */

#include "trace.h"
#include "query.h"

#define PE_RANGE_CHECK(p) IF_DEBUGGING(__shmem_pe_range_check(p))

/*
 * check the PE is within program allocation
 */
extern void __shmem_pe_range_check(int pe);

/*
 * how many elements in array T?
 *
 */
#define TABLE_SIZE(T) ( sizeof(T) / sizeof((T)[0]) )

#endif /* _UTILS_H */
