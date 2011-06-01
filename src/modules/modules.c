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

  snprintf(path_to_so, PATH_MAX, "%s/%s-%s.so", MODULES_DIR, group, name);

  mh = dlopen(path_to_so, RTLD_LAZY);
  if (mh == NULL) {
    return -1;
  }

  rh = (module_info_t *) dlsym(mh, "module_info");
  if (rh == NULL) {
    return -1;
  }

  (void) memcpy(mip, rh, sizeof(*mip));

  return 0;
}
