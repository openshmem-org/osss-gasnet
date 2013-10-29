#ifndef DEBUG_ALLOC_H
# define DEBUG_ALLOC_H 1

# ifdef HAVE_FEATURE_DEBUG

#  include <sys/types.h>

#  include "uthash.h"

typedef struct
{
  void *addr;		 /* key: shmalloc'ed address to be recorded */
  size_t size;		 /* how many bytes */
  UT_hash_handle hh;	 /* structure is hashable */
} alloc_table_t;

extern alloc_table_t *debug_alloc_new (void *a, size_t s);

extern void *debug_alloc_find (void *a);

extern void debug_alloc_add (void *a, size_t s);

extern void debug_alloc_del (void *a);

extern void debug_alloc_replace (void *a, size_t s);

extern void debug_alloc_dump (void);

#endif /* HAVE_FEATURE_DEBUG */

#endif /* DEBUG_ALLOC_H */
