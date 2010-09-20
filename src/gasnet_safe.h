#ifndef _GASNET_SAFE_H
#define _GASNET_SAFE_H 1

#include <gasnet.h>

#include "warn.h"

#define GASNET_SAFE(fncall) do {                                        \
    int _retval;                                                        \
    if ((_retval = fncall) != GASNET_OK) {                              \
      __shmem_warn(SHMEM_LOG_FATAL,					\
		   "error calling: %s at %s:%i, %s (%s)\n",		\
		   #fncall, __FILE__, __LINE__,				\
		   gasnet_ErrorName(_retval),				\
		   gasnet_ErrorDesc(_retval));				\
    }                                                                   \
  } while(0)

#endif /* _GASNET_SAFE_H */
