/* (c) 2011 University of Houston.  All rights reserved. */


#include <stdio.h>
#include <dlfcn.h>
#include <limits.h>
#include <string.h>

#include "trace.h"
#include "modules.h"

/*
 * TODO: currently keeping .so file open during run,
 * should really clean up
 */

int
__shmem_modules_load(const char *group, char *name, module_info_t *mip)
{
  void *mh;
  module_info_t *rh;
  char path_to_so[PATH_MAX];

  if (group == (char *) NULL) {
    return -1;
  }
  if (name == (char *) NULL) {
    return -1;
  }
  if (mip == (module_info_t *) NULL) {
    return -1;
  }

  snprintf(path_to_so, PATH_MAX, "%s/%s-%s.so",
           INSTALLED_MODULES_DIR, group, name);

  mh = dlopen(path_to_so, RTLD_LAZY);
  if (mh == NULL) {
    __shmem_trace(SHMEM_LOG_AUTH,
		  "internal error: couldn't open shared library \"%s\" (%s)",
		  path_to_so,
		  dlerror()
		  );
    return -1;
  }

  rh = (module_info_t *) dlsym(mh, "module_info");
  if (rh == NULL) {
    __shmem_trace(SHMEM_LOG_AUTH,
		  "internal error: couldn't find module_info symbol in \"%s\" (%s)",
		  path_to_so,
		  dlerror()
		  );
    return -1;
  }

  (void) memcpy(mip, rh, sizeof(*mip));

  /* (void) dlclose(mh); */

  return 0;
}
