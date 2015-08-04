/*
 *
 * Copyright (c) 2011 - 2015
 *   University of Houston System and UT-Battelle, LLC.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * o Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * o Neither the name of the University of Houston System, Oak Ridge
 *   National Laboratory nor the names of its contributors may be used to
 *   endorse or promote products derived from this software without specific
 *   prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include <gelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uthash.h"

#include "state.h"
#include "trace.h"
#include "exe.h"

#include "utils.h"

/*
 * ---------------------------------------------------------------------------
 *
 * handling lookups of global variables
 */

/*
 * map of global symbols
 *
 */

typedef struct
{
    void *addr;                 /* symbol's address is the key */
    char *name;                 /* name of symbol (for debugging) */
    size_t size;                /* bytes to represent symbol */
    UT_hash_handle hh;          /* structure is hashable */
} globalvar_t;

static globalvar_t *gvp = NULL; /* our global variable hash table */

/*
 * areas storing (uninitialized and initialized resp.) global
 * variables
 */

typedef struct
{
    size_t start;
    size_t end;
} global_area_t;

static global_area_t elfbss;    /* uninitialized */
static global_area_t elfdata;   /* initialized */
static global_area_t elfro;     /* read-only */


/*
 * scan the ELF image to build table of global symbold and the image
 * regions where they can be found (BSS and DATA)
 */
static inline int
table_init_helper (void)
{
    Elf *e = NULL;
    GElf_Ehdr ehdr;
    char *shstr_name = NULL;
    size_t shstrndx;
    Elf_Scn *scn = NULL;
    GElf_Shdr shdr;
    int ret = -1;
#if 0
    int (*getsi) ();            /* look up name of elf_get... routine */
#endif

    /* unrecognized format */
    if (elf_version (EV_CURRENT) == EV_NONE) {
        goto bail;
    }

    /* get the ELF object from already opened state */
    e = elf_begin (GET_STATE (exe_fd), ELF_C_READ, NULL);
    if (e == NULL) {
        goto bail;
    }

    /* do some sanity checks */
    if (elf_kind (e) != ELF_K_ELF) {
        goto bail;
    }
    if (gelf_getehdr (e, &ehdr) == NULL) {
        goto bail;
    }
    if (gelf_getclass (e) == ELFCLASSNONE) {
        goto bail;
    }

    /*
     * There are various elf_get* routines with different return values
     * in differnt libraries/versions - name of routine does not tell us
     * all we ned to know.  So let it go here, and we'll mop up any
     * problems later on.
     */

    /*
     * This routine is either "elf_getshdrstrndx" in newer ELF
     * libraries, or "elf_getshstrndx" in older ones.  Hard-code older
     * one for now since it is in the newer libraries, although marked as
     * deprecated.  This will be detected by autoconf later
     *
     * DEPRECATED
     */
    (void) elf_getshstrndx (e, &shstrndx);

    /* walk sections, look for RO/BSS/DATA and symbol table */
    scn = NULL;

    while ((scn = elf_nextscn (e, scn)) != NULL) {

        if (gelf_getshdr (scn, &shdr) != &shdr) {
            goto bail;
        }
        shstr_name = elf_strptr (e, shstrndx, shdr.sh_name);
        if (shstr_name == NULL) {
            goto bail;
        }

        /* found the read-only data */
        if (shdr.sh_type == SHT_PROGBITS && strcmp (shstr_name, ".rodata") == 0) {

            elfro.start = shdr.sh_addr;
            elfro.end = elfro.start + shdr.sh_size;

            shmemi_trace (SHMEM_LOG_SYMBOLS,
                          "ELF section .rodata for global variables = 0x%lX -> 0x%lX",
                          elfro.start, elfro.end);
            continue;           /* move to next scan */
        }

        /* found the uninitialized globals */
        if (shdr.sh_type == SHT_NOBITS && strcmp (shstr_name, ".bss") == 0) {

            elfbss.start = shdr.sh_addr;
            elfbss.end = elfbss.start + shdr.sh_size;

            shmemi_trace (SHMEM_LOG_SYMBOLS,
                          "ELF section .bss for global variables = 0x%lX -> 0x%lX",
                          elfbss.start, elfbss.end);
            continue;           /* move to next scan */
        }

        /* found the initialized globals */
        if (shdr.sh_type == SHT_PROGBITS && strcmp (shstr_name, ".data") == 0) {

            elfdata.start = shdr.sh_addr;
            elfdata.end = elfdata.start + shdr.sh_size;

            shmemi_trace (SHMEM_LOG_SYMBOLS,
                          "ELF section .data for global variables = 0x%lX -> 0x%lX",
                          elfdata.start, elfdata.end);
            continue;           /* move to next scan */
        }

        /* keep looking until we find the symbol table */
        if (shdr.sh_type == SHT_SYMTAB) {
            Elf_Data *data = NULL;
            while ((data = elf_getdata (scn, data)) != NULL) {
                GElf_Sym *es;
                GElf_Sym *last_es;

                es = (GElf_Sym *) data->d_buf;
                if (es == NULL) {
                    continue;
                }

                /* find out how many entries to look for */
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
                    /*
                     * this macro handles a symbol that is present
                     * in one libelf implementation but isn't in another
                     * (elfutils vs. libelf)
                     */
#ifndef GELF_ST_VISIBILITY
#define GELF_ST_VISIBILITY(o) ELF64_ST_VISIBILITY(o)
#endif
                    if (GELF_ST_TYPE (es->st_info) != STT_OBJECT &&
                        GELF_ST_VISIBILITY (es->st_info) != STV_DEFAULT) {
                        continue;
                    }
                    name = elf_strptr (e, shdr.sh_link, (size_t) es->st_name);
                    if (name == NULL || *name == '\0') {
                        continue;
                    }
                    /* put the symbol and info into the symbol hash table */
                    {
                        globalvar_t *gv = (globalvar_t *) malloc (sizeof (*gv));
                        if (gv == NULL) {
                            goto bail;
                        }
                        gv->name = strdup (name);
                        if (gv->name == NULL) {
                            free (gv);
                            goto bail;
                        }
                        gv->addr = (void *) es->st_value;
                        gv->size = es->st_size;
                        HASH_ADD_PTR (gvp, addr, gv);
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
    }

  bail:

    if (elf_end (e) != 0) {
        ret = -1;
    }

    return ret;
}

/* ======================================================================== */

#if 0

/*
 * helpers for debug output: not used currently
 */
static int
addr_sort (globalvar_t * a, globalvar_t * b)
{
    return ((char *) (a->addr) - (char *) (b->addr));
}

static void
print_global_var_table (shmem_trace_t msgtype)
{
    globalvar_t *g;
    globalvar_t *tmp;

    if (!shmemi_trace_is_enabled (msgtype)) {
        return;
    }

    shmemi_trace (msgtype, "-- start hash table --");

    HASH_SORT (gvp, addr_sort);

    HASH_ITER (hh, gvp, g, tmp) {
        shmemi_trace (msgtype,
                      "address %p: name \"%s\", size %ld",
                      g->addr, g->name, g->size);
    }

    shmemi_trace (msgtype, "-- end hash table --");
}

#endif /* 0: unused debugging routines */

/*
 * read in the symbol table and global data areas
 */
void
shmemi_symmetric_globalvar_table_init (void)
{
    if (table_init_helper () != 0) {
        shmemi_trace (SHMEM_LOG_FATAL,
                      "internal error: couldn't read global symbols in executable");
        /* NOT REACHED */
    }

    /* print_global_var_table(SHMEM_LOG_SYMBOLS); */
}

/*
 * free hash table here
 */
void
shmemi_symmetric_globalvar_table_finalize (void)
{
    globalvar_t *current;
    globalvar_t *tmp;

    HASH_ITER (hh, gvp, current, tmp) {
        free (current->name);   /* was strdup'ed above */
        HASH_DEL (gvp, current);
        free (current);
    }
}

/*
 * helper to check address ranges
 */
#define IN_RANGE(Area, Addr)                                \
  ( ( (Area).start <= (Addr) ) && ( (Addr) < (Area).end ) )

#define IS_GLOBAL(Addr)                                                 \
  (IN_RANGE (elfdata, a) || IN_RANGE (elfbss, a) || IN_RANGE (elfro, a))

/*
 * check to see if address is global
 */
int
shmemi_symmetric_is_globalvar (void *addr)
{
    const size_t a = (size_t) addr;

    if (EXPR_LIKELY (IS_GLOBAL (a))) {
        return 1;
    }
    else {
        return 0;
    }
}
