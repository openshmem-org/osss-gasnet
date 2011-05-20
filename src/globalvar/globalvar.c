#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <gelf.h>
#include <libelf.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <values.h>

#include "uthash.h"

#include "state.h"
#include "trace.h"
#include "exe.h"

/*
 * ---------------------------------------------------------------------------
 *
 * handling lookups of global variables
 */

/*
 * map of global symbols
 *
 */

typedef struct {
  void *addr;                   /* symbol's address is the key */
  char *name;                   /* name of symbol (for debugging) */
  size_t size;                  /* bytes to represent symbol */
  UT_hash_handle hh;            /* structure is hashable */
} globalvar_t;

static globalvar_t *gvp = NULL; /* our global variable hash table */

/*
 * areas storing (uninitialized and initialized resp.) global
 * variables
 */

typedef struct {
  size_t start;
  size_t end;
} global_area_t;

static global_area_t elfbss;
static global_area_t elfdata;


/*
 * scan the ELF image to build table of global symbold and the image
 * regions where they can be found (BSS and DATA)
 */
static int
table_init_helper(void)
{
  Elf *e;
  GElf_Ehdr ehdr;
  char *shstr_name;
  size_t shstrndx;
  Elf_Scn *scn;
  GElf_Shdr shdr;
  Elf_Data *data;
  int ret = -1;

  /* unrecognized format */
  if (elf_version(EV_CURRENT) == EV_NONE) {
    goto bail;
  }

  /* get the ELF object from already opened state */
  e = elf_begin(GET_STATE(exe_fd), ELF_C_READ, NULL);
  if (e == NULL) {
    goto bail;
  }

  /* do some sanity checks */
  if (elf_kind(e) != ELF_K_ELF) {
    goto bail;
  }
  if (gelf_getehdr(e, &ehdr) == NULL) {
    goto bail;
  }
  if (gelf_getclass(e) == ELFCLASSNONE) {
    goto bail;
  }
  /* TODO: deprecated interface from older libelf on CentOS */
  if (elf_getshstrndx(e, &shstrndx) != 0) {
    goto bail;
  }

  /* walk sections, look for BSS/DATA and symbol table */
  scn = NULL;

  while ((scn = elf_nextscn(e, scn)) != NULL) {

    if (gelf_getshdr(scn, &shdr) != &shdr) {
      goto bail;
    }
    if ((shstr_name = elf_strptr(e, shstrndx, shdr.sh_name)) == NULL) {
      goto bail;
    }

    /* found the .bss section */
    if (shdr.sh_type == SHT_NOBITS &&
	strcmp(shstr_name, ".bss") == 0) {

      elfbss.start = shdr.sh_addr;
      elfbss.end = elfbss.start + shdr.sh_size;

      __shmem_trace(SHMEM_LOG_SYMBOLS,
		    "ELF section .bss for global variables = 0x%lX -> 0x%lX",
		    elfbss.start, elfbss.end
		    );
      continue;			/* move to next scan */
    }

    /* found the .data section */
    if (shdr.sh_type == SHT_PROGBITS &&
	strcmp(shstr_name, ".data") == 0) {

      elfdata.start = shdr.sh_addr;
      elfdata.end = elfdata.start + shdr.sh_size;

      __shmem_trace(SHMEM_LOG_SYMBOLS,
		    "ELF section .data for global variables = 0x%lX -> 0x%lX",
		    elfdata.start, elfdata.end
		    );
      continue;			/* move to next scan */
    }

    /* keep looking until we find the symbol table */
    if (shdr.sh_type != SHT_SYMTAB) {
      continue;
    }

    /* found valid-looking symbol table */
    data = NULL;
    while ((data = elf_getdata(scn, data)) != NULL) {
      GElf_Sym *es;
      GElf_Sym *last_es;

      es = (GElf_Sym *) data->d_buf;
      if (es == NULL) {
	continue;
      }

      last_es = (GElf_Sym *) ((char *) data->d_buf + data->d_size);

      for (; es < last_es; es += 1) {
	char *name;

	/*
	 * need visible global or local (Fortran save) object with
	 * some kind of content
	 */
	if (es->st_value == 0 || es->st_size == 0) {
	  continue;
	}
	if (GELF_ST_TYPE(es->st_info) != STT_OBJECT &&
	    GELF_ST_VISIBILITY(es->st_info) != STV_DEFAULT) {
	  continue;
	}
	name = elf_strptr(e, shdr.sh_link, (size_t) es->st_name);
	if (name == NULL || *name == '\0') {
	  continue;
	}
	{
	  globalvar_t *gv = (globalvar_t *) malloc(sizeof(*gv));
	  if (gv == NULL) {
	    goto bail;
	  }
	  gv->addr = (void *) es->st_value;
	  gv->size = es->st_size;
	  gv->name = strdup(name);
	  HASH_ADD_PTR(gvp, addr, gv);
	}
      }
    }
    /*
     * pulled out all the global symbols => success,
     * don't need to scan further
     */
    ret = 0;
    break;
  }

 bail:

  if (elf_end(e) != 0) {
    ret = -1;
  }

  return ret;
}

/* ======================================================================== */

/*
 * helpers for debug output: not used currently
 */
static int
addr_sort(globalvar_t *a, globalvar_t *b)
{
  return ( (char *) (a->addr) - (char *) (b->addr) );
}

static void
print_global_var_table(shmem_trace_t msgtype)
{
  globalvar_t *g;
  globalvar_t *tmp;

  if (! __shmem_trace_is_enabled(msgtype)) {
    return;
  }

  __shmem_trace(msgtype,
		"-- start hash table --"
		);

  HASH_SORT(gvp, addr_sort);

  HASH_ITER(hh, gvp, g, tmp) {
    __shmem_trace(msgtype,
		  "address %p: name \"%s\", size %ld",
		  g->addr, g->name, g->size
		  );
  }

  __shmem_trace(msgtype,
		"-- end hash table --"
		);
}

/*
 * read in the symbol table and global data areas
 */
void
__shmem_symmetric_globalvar_table_init(void)
{
  if (table_init_helper() != 0) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: couldn't read global symbols in executable"
		  );
    /* NOT REACHED */
  }

  /* too noisy: print_global_var_table(SHMEM_LOG_SYMBOLS); */
}

/*
 * free hash table here
 */
static void
__shmem_symmetric_globalvar_table_finalize(void)
{
  globalvar_t *current;
  globalvar_t *tmp;

  HASH_ITER(hh, gvp, current, tmp) {
      HASH_DEL(gvp, current);
      free(current);
  }
}

/*
 * helper to check address ranges
 */
#define IN_RANGE(Area, Addr) \
  ( ( Area.start <= (Addr) ) && ( (Addr) < Area.end ) )

/*
 * check to see if address is global
 */
int
__shmem_symmetric_is_globalvar(void *addr)
{
  size_t a = (size_t) addr;

  return IN_RANGE(elfbss, a) || IN_RANGE(elfdata, a);
}
