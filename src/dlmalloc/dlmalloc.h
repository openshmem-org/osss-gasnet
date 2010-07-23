#include <sys/types.h>

#define ONLY_MSPACES 1
#define USE_LOCKS 1

typedef void *mspace;
extern mspace create_mspace_with_base(void* base, size_t capacity, int locked);
extern size_t destroy_mspace(mspace msp);
extern void * mspace_malloc(mspace msp, size_t bytes);
extern void* mspace_realloc(mspace msp, void* mem, size_t newsize);
extern void* mspace_memalign(mspace msp, size_t alignment, size_t bytes);
extern void mspace_free(mspace msp, void *mem);
extern size_t mspace_footprint(mspace msp);
