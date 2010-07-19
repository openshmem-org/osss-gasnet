#ifndef _GASNET_SAFE_H
#define _GASNET_SAFE_H 1

#include <gasnet.h>

#define GASNET_SAFE(fncall) do {                                        \
    int _retval;                                                        \
    if ((_retval = fncall) != GASNET_OK) {                              \
      fprintf(stderr, "ERROR calling: %s\n"                             \
              " at: %s:%i\n"                                            \
              " error: %s (%s)\n",                                      \
              #fncall, __FILE__, __LINE__,                              \
              gasnet_ErrorName(_retval), gasnet_ErrorDesc(_retval));    \
      fflush(stderr);                                                   \
      gasnet_exit(_retval);                                             \
    }                                                                   \
  } while(0)

#endif /* _GASNET_SAFE_H */
