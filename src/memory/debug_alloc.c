#ifdef HAVE_FEATURE_DEBUG

# include "uthash.h"

# include "trace.h"

# include "debug_alloc.h"

static alloc_table_t *atp = NULL; /* our allocation hash table */

alloc_table_t *
debug_alloc_new (void *a, size_t s)
{
  alloc_table_t *at = (alloc_table_t *) malloc (sizeof (*at));

  if (at == (alloc_table_t *) NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: out of memory allocating address/size record"
		     );
      /* NOT REACHED */
    }
  at->addr = a;
  at->size = s;
  return at;
}

void *
debug_alloc_find (void *a)
{
  alloc_table_t *at = NULL;	/* entry corresponding to "a" */

  HASH_FIND_PTR (atp, &a, at);
  return at;
}

void
debug_alloc_add (void *a, size_t s)
{
  alloc_table_t *at = debug_alloc_new (a, s);

  HASH_ADD_PTR (atp, addr, at);
}

void
debug_alloc_del (void *a)
{
  alloc_table_t *at = debug_alloc_find (a);

  if (at == (alloc_table_t *) NULL)
    {
      __shmem_trace (SHMEM_LOG_FATAL,
		     "internal error: no hash table entry for address %p",
		     a
		     );
      /* NOT REACHED */
    }
  HASH_DEL (atp, at);

  free (at);
}

void
debug_alloc_replace (void *a, size_t s)
{
#if 0
  alloc_table_t *at = debug_alloc_new (a, s);
  HASH_REPLACE_PTR (atp, a, at);
#else
  debug_alloc_del (a);
  debug_alloc_add (a, s);
#endif
}

void
debug_alloc_dump (void)
{
  alloc_table_t *tmp;
  alloc_table_t *s;

  HASH_ITER (hh, atp, s, tmp) {
    __shmem_trace (SHMEM_LOG_MEMORY,
		   "addr = %p, size = %ld", s->addr, s->size
		   );
  }
}

#endif /* HAVE_FEATURE_DEBUG */
