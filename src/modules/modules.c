/* (c) 2011 University of Houston System.  All rights reserved. */


#include <stdio.h>
#include <dlfcn.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include "trace.h"
#include "modules.h"

/*
 * read in the config file and set up tables
 *
 */

#include "uthash.h"

typedef struct {
  char *name;                   /* module = key */

  int lineno;                   /* line in the config file */
  char *impl;                   /* name of implementation */

  UT_hash_handle hh;

} module_table_t;

static module_table_t *mtp = NULL;

static char *fallback_algorithm = "naive";

/*
 * pretend to be Perl: get rid of newline
 */
#define CHOMP(L) (L)[strlen(L) - 1] = '\0'

/*
 * read in config file and build table of module/algorithm lookups
 *
 */

static void
create_module_table_from_config(char *cfg_file)
{
  FILE *fp;
  char *modname;
  char *modimpl;
  char *hash;
  int lineno = 0;
  const char *delims = "=\t ";	/* equal, tab, space */
  char line[PATH_MAX];

  fp = fopen(cfg_file, "r");
  if (fp == (FILE *) NULL) {
    return;
  }

  __shmem_trace(SHMEM_LOG_MODULES,
		"about to read modules from \"%s\"",
		cfg_file
		);

  while (fgets(line, sizeof(line), fp) != NULL) {
    lineno += 1;

    /*
     * trim blank lines and comments
     */
    CHOMP(line);
    hash = strchr(line, '#');
    if (hash != (char *) NULL) {
      *hash = '\0';
    }

    modname = strtok(line, delims);
    if (modname == (char *) NULL) {
      continue;
    }

    modimpl = strtok((char *) NULL, delims);
    if (modimpl == (char *) NULL) {
      __shmem_trace(SHMEM_LOG_MODULES,
		    "no implementation for module \"%s\" in \"%s\" at line %d, "
		    "will use default",
		    modname,
		    cfg_file,
		    lineno
		    );
      continue;
    }

    {    
      module_table_t *mp = (module_table_t *) malloc(sizeof(*mp));
      if (mp == (module_table_t *) NULL) {
	__shmem_trace(SHMEM_LOG_FATAL,
		      "internal error: unable to allocate memory for module table"
		      );
	/* NOT REACHED */
      }
      mp->lineno = lineno;
      mp->name = strdup(modname);
      mp->impl = strdup(modimpl);

      HASH_ADD_KEYPTR(hh, mtp, mp->name, strlen(mp->name), mp);
    }

    __shmem_trace(SHMEM_LOG_MODULES,
		  "module \"%s\", implementation \"%s\" in \"%s\" at line %d",
		  modname,
		  modimpl,
		  cfg_file,
		  lineno
		  );
  } /* end line parser */

  (void) fclose(fp);
}

/*
 * clean up hash table when shutting down
 *
 */

static void
free_module_table(void)
{
  module_table_t *current;
  module_table_t *tmp;

  HASH_ITER(hh, mtp, current, tmp) {
    HASH_DEL(mtp, current);
    free(current);
  }
}

/*
 * retrieve implementation for a given module, or the default
 * if nothing specific given
 *
 */
char *
__shmem_modules_get_implementation(char *mod)
{
  module_table_t *match;

  HASH_FIND_STR(mtp, mod, match);
  if (match != (module_table_t *) NULL) {
    return match->impl;
  }

  HASH_FIND_STR(mtp, "default", match);
  if (match != (module_table_t *) NULL) {
    return match->impl;
  }

  return fallback_algorithm;
}

/*
 * initialize modules: read from global config file if it exists
 *
 */

void
__shmem_modules_init(void)
{
  char path_to_cfg[PATH_MAX];

  snprintf(path_to_cfg, PATH_MAX,
	   "%s/config",
	   INSTALLED_MODULES_DIR
	   );

  create_module_table_from_config(path_to_cfg);
}

/*
 * shut modules down
 *
 */

void
__shmem_modules_finalize(void)
{
  free_module_table();
}

/*
 * TODO: currently keeping .so file open during run,
 * should really clean up
 *
 * return 0 if module located and info filled out, -1 if problem
 */

int
__shmem_modules_load(const char *group, char *name, module_info_t *mip)
{
  void *mh;
  module_info_t *rh;
  char path_to_so[PATH_MAX];

  /*
   * sanity-check the request
   */
  if (group == (char *) NULL) {
    return -1;
  }
  if (name == (char *) NULL) {
    return -1;
  }
  if (mip == (module_info_t *) NULL) {
    return -1;
  }

  /*
   * locate the .so file
   */
  snprintf(path_to_so, PATH_MAX,
	   "%s/%s-%s.so",
           INSTALLED_MODULES_DIR,
	   group,
	   name
	   );

  mh = dlopen(path_to_so, RTLD_LAZY);
  if (mh == NULL) {
    __shmem_trace(SHMEM_LOG_AUTH,
		  "internal error: couldn't open shared library \"%s\" (%s)",
		  path_to_so,
		  dlerror()
		  );
    return -1;
  }

  /*
   * pull out & save the routine lookup structure
   */
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
