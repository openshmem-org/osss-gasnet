/*
 * This file provides the layer on top of GASNet, ARMCI or whatever.
 * API should be formalized at some point, but basically everything
 * non-static that starts with "__comms_"
 */

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <values.h>
#include <setjmp.h>

#include "gasnet_safe.h"

#include "state.h"
#include "memalloc.h"
#include "trace.h"
/* #include "dispatch.h" */
#include "atomic.h"
#include "comms.h"
#include "ping.h"


/*
 * gasnet model choice
 *
 */

#if defined(GASNET_SEGMENT_FAST) || defined(GASNET_SEGMENT_LARGE)
#  define HAVE_MANAGED_SEGMENTS 1
#elif defined(GASNET_SEGMENT_EVERYTHING)
#  undef HAVE_MANAGED_SEGMENTS
#else
#  error "I don't know what kind of GASNet segment model you're trying to use"
#endif

static gasnet_seginfo_t *seginfo_table;

#if ! defined(HAVE_MANAGED_SEGMENTS)

/*
 * this will be malloc'ed so we can respect setting from environment
 * variable
 */

#define DEFAULT_HEAP_SIZE 10000000L

static void *great_big_heap;

#endif /* ! HAVE_MANAGED_SEGMENTS */

/*
 * define accepted size units in ascending order, which are
 * S.I. compliant
 *
 * http://en.wikipedia.org/wiki/SI_Unit_Prefixes
 *
 */

static char *units_string = "kmgtpezy";

static const size_t multiplier = 1000L;

/*
 * work out how big the symmetric segment areas should be.
 *
 * Either from environment setting, or default value from
 * implementation
 */
static size_t
__comms_get_segment_size(void)
{
  char unit = '\0';
  size_t bytes = 1L;
  char *p;
  char *mlss_str = __comms_getenv("SHMEM_SYMMETRIC_HEAP_SIZE");

  if (mlss_str == (char *) NULL) {
#ifdef HAVE_MANAGED_SEGMENTS
    return (size_t) gasnet_getMaxLocalSegmentSize();
#else
    return DEFAULT_HEAP_SIZE;
#endif
  }

  p = mlss_str;
  while (*p != '\0') {
    if (! isdigit(*p)) {
      unit = *p;
      *p = '\0';		/* get unit, chop */
      break;
    }
    p += 1;
  }

  /* if there's a unit, work out how much to scale */
  if (unit != '\0') {
    int i;
    int foundit = 0;
    char *usp = units_string;

    unit = tolower(unit);
    while (*usp != '\0') {
      bytes *= multiplier;
      if (*usp == unit) {
	foundit = 1;
	break;
      }
      usp += 1;
    }

    if (! foundit) {
      /* don't know that unit! */
      __shmem_trace(SHMEM_LOG_FATAL,
		    "unknown data size unit \"%c\" in symmetric heap specification",
		    unit);
      /* NOT REACHED */
    }
  }

  return bytes * (size_t) strtol(mlss_str, (char **) NULL, 10);
}

/*
 * allow the runtime to change the spin/block behavior dynamically,
 * would allow adaptivity
 */
void
__comms_set_waitmode(comms_spinmode_t mode)
{
  int gm;
  const char *mstr;

  switch (mode) {
  case SHMEM_COMMS_SPINBLOCK:
    gm = GASNET_WAIT_SPINBLOCK;
    mstr = "spinblock";
    break;
  case SHMEM_COMMS_SPIN:
    gm = GASNET_WAIT_SPIN;
    mstr = "spin";
    break;
  case SHMEM_COMMS_BLOCK:
    gm = GASNET_WAIT_BLOCK;
    mstr = "block";
    break;
  default:
    __shmem_trace(SHMEM_LOG_FATAL,
		  "tried to set unknown wait mode %d",
		  (int) mode
		  );
    /* NOT REACHED */
    break;
  }

  GASNET_SAFE( gasnet_set_waitmode(gm) );

  __shmem_trace(SHMEM_LOG_DEBUG,
		"set waitmode to %s",
		mstr);
}

/*
 * used in service thread to poll for put/get/AM traffic
 */
void
__comms_poll(void)
{
  gasnet_AMPoll();
}

/*
 * used in loops while waiting for variable to change
 */

void
__comms_pause(void)
{
  pthread_yield();
  // __asm__ __volatile__("rep;nop": : :"memory");
}

/*
 * As Arnie said, GET...OUT...
 */
void
__comms_exit(int status)
{
  gasnet_exit(status);
}

/*
 * make sure everyone finishes stuff, then exit.
 */
void
__comms_shutdown(int status)
{
  __comms_barrier_all();
  __comms_exit(status);
}

/*
 * can't just call getenv, it might not pass through environment
 * info to other nodes from launch.
 */
char *
__comms_getenv(const char *name)
{
  return gasnet_getenv(name);
}

/*
 * which node (PE) am I?
 */
int
__comms_mynode(void)
{
  return (int) gasnet_mynode();
}

/*
 * how many nodes (Pes) take part in this program?
 */
int
__comms_nodes(void)
{
  return (int) gasnet_nodes();
}

/*
 * we use the _nbi routine, so that gasnet tracks outstanding
 * I/O for us (fence/barrier waits for these implicit handles)
 */
void
__comms_put(void *dst, void *src, size_t len, int pe)
{
  gasnet_put_nbi(pe, dst, src, len);
}

void
__comms_get(void *dst, void *src, size_t len, int pe)
{
  gasnet_get(dst, pe, src, len);
}

/*
 * not completely sure about using longs in these two:
 * it's big enough and hides the gasnet type: is that good enough?
 */

void
__comms_put_val(void *dst, long src, size_t len, int pe)
{
  gasnet_put_nbi_val(pe, dst, src, len);
}

long
__comms_get_val(void *src, size_t len, int pe)
{
  return gasnet_get_val(pe, src, len);
}

#define COMMS_TYPE_PUT_NB(Name, Type)					\
  void *								\
  __comms_##Name##_put_nb(Type *target, Type *source, size_t len, int pe) \
  {									\
    return gasnet_put_nb(pe, target, source, sizeof(Type) * len);	\
  }

COMMS_TYPE_PUT_NB(short, short)
COMMS_TYPE_PUT_NB(int, int)
COMMS_TYPE_PUT_NB(long, long)
COMMS_TYPE_PUT_NB(longdouble, long double)
COMMS_TYPE_PUT_NB(longlong, long long)
COMMS_TYPE_PUT_NB(double, double)
COMMS_TYPE_PUT_NB(float, float)

#pragma weak __comms_putmem_nb = __comms_long_put_nb

void
__comms_wait_nb(void *h)
{
  gasnet_wait_syncnb((gasnet_handle_t) h);
  LOAD_STORE_FENCE();
}

/*
 * TODO: this should be OK for quiet.  Is it overkill?  Correct?  Is
 * there a better way?
 *
 */

void
__comms_quiet(void)
{
  __comms_barrier_all();
}

void
__comms_fence(void)
{
  gasnet_wait_syncnbi_all();
  LOAD_STORE_FENCE();
}

static int barcount = 0;
static int barflag = 0;

void
__comms_barrier_all(void)
{
  /* GASNET_BEGIN_FUNCTION(); */

  /* wait for gasnet to finish pending puts/gets */
  __comms_fence();

  /* use gasnet's global barrier */
  gasnet_barrier_notify(barcount, barflag);
  GASNET_SAFE( gasnet_barrier_wait(barcount, barflag) );

  /* barcount = 1 - barcount; */
  barcount += 1;

  __comms_fence();
}


/*
 * ---------------------------------------------------------------------------
 *
 * handling lookups of global variables
 */

/*
 * map of global symbols
 *
 */
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <libelf.h>
#include <gelf.h>
#include "uthash.h"

typedef struct {
  void *addr;                   /* symbol's address is the key */
  char *name;                   /* name of symbol (for debugging) */
  size_t size;                  /* bytes to represent symbol */
  UT_hash_handle hh;            /* structure is hashable */
} globalvar_t;

static globalvar_t *gvp = NULL; /* our hash table */

/*
 * areas storing (uninitialized and initialized resp.) global
 * variables
 */
static size_t bss_start;
static size_t bss_end;

static size_t data_start;
static size_t data_end;

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
  int fd;
  int ret = -1;

  fd = open("/proc/self/exe", O_RDONLY, 0);
  if (fd < 0) {
    return ret;
  }

  /* unrecognized format */
  if (elf_version(EV_CURRENT) == EV_NONE) {
    goto bail;
  }

  /* get the ELF object */
  e = elf_begin(fd, ELF_C_READ, NULL);
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
  if (elf_getshstrndx(e, &shstrndx) != 0) {
    goto bail;
  }

  /* walk sections, look for BSS and symbol table */
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

      bss_start = shdr.sh_addr;
      bss_end = bss_start + shdr.sh_size;

      __shmem_trace(SHMEM_LOG_SYMBOLS,
		    "ELF section .bss for global variables = 0x%lX -> 0x%lX",
		    bss_start, bss_end
		    );

      continue;
    }

    /* found the .data section */
    if (shdr.sh_type == SHT_PROGBITS &&
	strcmp(shstr_name, ".data") == 0) {

      data_start = shdr.sh_addr;
      data_end = data_start + shdr.sh_size;

      __shmem_trace(SHMEM_LOG_SYMBOLS,
		    "ELF section .data for global variables = 0x%lX -> 0x%lX",
		    data_start, data_end
		    );

      continue;
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
  }
  /* pulled out all the global symbols => success */
  ret = 0;

 bail:

  /* let's be relaxed about whether these succeed */
  elf_end(e);
  close(fd);

  return ret;
}

/* ======================================================================== */

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

  if (! __trace_is_enabled(msgtype)) {
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

static void
__comms_globalvar_table_init(void)
{
  if (table_init_helper() != 0) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: could'nt read global symbols in executable"
		  );
    /* NOT REACHED */
  }

  print_global_var_table(SHMEM_LOG_SYMBOLS);
}

static void
__comms_globalvar_table_finalize(void)
{
  /* could free hash table here */
}

#if 0
int
__comms_is_globalvar(void *addr)
{
  globalvar_t *gp;

  HASH_FIND_PTR(gvp, &addr, gp);

  return (gp != NULL);
}
#endif

static
int
__comms_is_bss(size_t a)
{
  return (bss_start <= a) && (a <= bss_end);
}

static
int
__comms_is_data(size_t a)
{
  return (data_start <= a) && (a <= data_end);
}

int
__comms_is_globalvar(void *addr)
{
  size_t a = (size_t) addr;

  return __comms_is_bss(a) || __comms_is_data(a);
}

/*
 * ---------------------------------------------------------------------------
 *
 * start of handlers
 */

#if ! defined(HAVE_MANAGED_SEGMENTS)

#define GASNET_HANDLER_SETUP_OUT     128
#define GASNET_HANDLER_SETUP_BAK     129

static void handler_segsetup_out();
static void handler_segsetup_bak();

#endif /* ! HAVE_MANAGED_SEGMENTS */

#define GASNET_HANDLER_SWAP_OUT      132
#define GASNET_HANDLER_SWAP_BAK      133

static void handler_swap_out();
static void handler_swap_bak();

#define GASNET_HANDLER_CSWAP_OUT     134
#define GASNET_HANDLER_CSWAP_BAK     135

static void handler_cswap_out();
static void handler_cswap_bak();

#define GASNET_HANDLER_FADD_OUT      136
#define GASNET_HANDLER_FADD_BAK      137

static void handler_fadd_out();
static void handler_fadd_bak();

#define GASNET_HANDLER_FINC_OUT      138
#define GASNET_HANDLER_FINC_BAK      139

static void handler_finc_out();
static void handler_finc_bak();

#define GASNET_HANDLER_ADD_OUT       140
#define GASNET_HANDLER_ADD_BAK       141

static void handler_add_out();
static void handler_add_bak();

#define GASNET_HANDLER_INC_OUT       142
#define GASNET_HANDLER_INC_BAK       143

static void handler_inc_out();
static void handler_inc_bak();

#define GASNET_HANDLER_PING_OUT      144
#define GASNET_HANDLER_PING_BAK      145

static void handler_ping_out();
static void handler_ping_bak();

#if defined(HAVE_MANAGED_SEGMENTS)

#define GASNET_HANDLER_GLOBALVAR_OUT 130
#define GASNET_HANDLER_GLOBALVAR_BAK 131

static void handler_globalvar_out();
static void handler_globalvar_bak();

#endif /* HAVE_MANAGED_SEGMENTS */

static gasnet_handlerentry_t handlers[] =
  {
#if ! defined(HAVE_MANAGED_SEGMENTS)
    { GASNET_HANDLER_SETUP_OUT,     handler_segsetup_out  },
    { GASNET_HANDLER_SETUP_BAK,     handler_segsetup_bak  },
#endif /* ! HAVE_MANAGED_SEGMENTS */
    { GASNET_HANDLER_SWAP_OUT,      handler_swap_out      },
    { GASNET_HANDLER_SWAP_BAK,      handler_swap_bak      },
    { GASNET_HANDLER_CSWAP_OUT,     handler_cswap_out     },
    { GASNET_HANDLER_CSWAP_BAK,     handler_cswap_bak     },
    { GASNET_HANDLER_FADD_OUT,      handler_fadd_out      },
    { GASNET_HANDLER_FADD_BAK,      handler_fadd_bak      },
    { GASNET_HANDLER_FINC_OUT,      handler_finc_out      },
    { GASNET_HANDLER_FINC_BAK,      handler_finc_bak      },
    { GASNET_HANDLER_ADD_OUT,       handler_add_out       },
    { GASNET_HANDLER_ADD_BAK,       handler_add_bak       },
    { GASNET_HANDLER_INC_OUT,       handler_inc_out       },
    { GASNET_HANDLER_INC_BAK,       handler_inc_bak       },
    { GASNET_HANDLER_PING_OUT,      handler_ping_out      },
    { GASNET_HANDLER_PING_BAK,      handler_ping_bak      },
#if defined(HAVE_MANAGED_SEGMENTS)
    { GASNET_HANDLER_GLOBALVAR_OUT, handler_globalvar_out },
    { GASNET_HANDLER_GLOBALVAR_BAK, handler_globalvar_bak },
#endif /* HAVE_MANAGED_SEGMENTS */
  };
static const int nhandlers = sizeof(handlers) / sizeof(handlers[0]);

/*
 * end of handlers
 */

/*
 * This is where the communications layer gets set up
 */
void
__comms_init(void)
{
  /*
   * fake the command-line args
   */
  int argc = 1;
  char **argv;

  argv = (char **) malloc(argc * sizeof(*argv));
  if (argv == (char **) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "could not allocate memory for GASNet initialization"
		  );
    /* NOT REACHED */
  }
  argv[0] = "shmem";
  
  GASNET_SAFE( gasnet_init(&argc, &argv) );

  /*
   * now we can ask about the node count & heap
   */
  __state.mype     = __comms_mynode();
  __state.numpes   = __comms_nodes();
  __state.heapsize = __comms_get_segment_size();

  /*
   * not guarding the attach for different gasnet models,
   * since last 2 params are ignored if not needed
   */
  GASNET_SAFE(
	      gasnet_attach(handlers, nhandlers,
			    __state.heapsize, 0
			    )
	      );

  __comms_set_waitmode(SHMEM_COMMS_SPINBLOCK);

  __comms_globalvar_table_init();

  /*
   * make sure all nodes are up to speed before "declaring"
   * initialization done
   */
  __comms_barrier_all();

  __shmem_trace(SHMEM_LOG_INIT,
		"communication layer initialization complete"
		);

  /* Up and running! */
}

/*
 * ---------------------------------------------------------------------------
 *
 * initialize the symmetric segments.
 *
 * In the gasnet fast/large models, use the attached segments and
 * manage address translations through the segment table
 *
 * In the everything model, we allocate on our own heap and send out
 * the addresses with active messages
 */

#if ! defined(HAVE_MANAGED_SEGMENTS)

/*
 * remotely modified, stop it being put in a register
 */
static volatile int seg_setup_replies_received = 0;

static gasnet_hsl_t setup_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t setup_bak_lock = GASNET_HSL_INITIALIZER;

/*
 * unpack buf from sender PE and store seg info locally.  Ack. receipt.
 */
static void
handler_segsetup_out(gasnet_token_t token,
		     void *buf, size_t bufsiz,
		     gasnet_handlerarg_t unused)
{
  gasnet_node_t src_pe;
  gasnet_seginfo_t *gsp = (gasnet_seginfo_t *) buf;

  /*
   * no lock here: each PE writes exactly once to its own array index,
   * and only to that...
   */

  /* gasnet_hsl_lock(& setup_out_lock); */

  GASNET_SAFE( gasnet_AMGetMsgSource(token, &src_pe) );

  seginfo_table[(int) src_pe].addr = gsp->addr;
  seginfo_table[(int) src_pe].size = gsp->size;

  /* gasnet_hsl_unlock(& setup_out_lock); */

  gasnet_AMReplyMedium1(token, GASNET_HANDLER_SETUP_BAK,
			(void *) NULL, 0, unused);
}

/*
 * record receipt ack.  We only need to count the number of replies
 */
static void
handler_segsetup_bak(gasnet_token_t token,
		     void *buf, size_t bufsiz,
		     gasnet_handlerarg_t unused)
{
  gasnet_hsl_lock(& setup_bak_lock);

  seg_setup_replies_received += 1;

  gasnet_hsl_unlock(& setup_bak_lock);
}

#endif /* ! HAVE_MANAGED_SEGMENTS */

void
__symmetric_memory_init(void)
{
  /*
   * calloc zeroes for us
   */
  seginfo_table = (gasnet_seginfo_t *) calloc(__state.numpes,
					      sizeof(gasnet_seginfo_t));
  if (seginfo_table == (gasnet_seginfo_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "could not allocate GASNet segments (%s)",
		  strerror(errno)
		  );
    /* NOT REACHED */
  }

  /*
   * prep the segments for use across all PEs
   *
   * each PE manages its own segment, but can see addresses from all PEs
   */

#ifdef HAVE_MANAGED_SEGMENTS

  GASNET_SAFE( gasnet_getSegmentInfo(seginfo_table, __state.numpes) );

#else

  /* allocate the heap - has to be pagesize aligned */
  if (posix_memalign(& great_big_heap,
		     GASNET_PAGESIZE, __state.heapsize) != 0) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "unable to allocate symmetric heap"
		  );
    /* NOT REACHED */
  }

  /*
   * need to make sure everyone has segment table allocated before
   * exchanging messages
   */
  __comms_barrier_all();

  __shmem_trace(SHMEM_LOG_MEMORY,
		"symmetric heap @ %p, size is %ld bytes",
		great_big_heap, __state.heapsize
		);

  {
    gasnet_seginfo_t gsp;
    int pe;

    for (pe = 0; pe < __state.numpes; pe += 1) {
      /* send to everyone else */
      if (__state.mype != pe) {

	gsp.addr = great_big_heap;
        gsp.size = __state.heapsize;

	gasnet_AMRequestMedium1(pe, GASNET_HANDLER_SETUP_OUT,
				&gsp, sizeof(gsp),
				0);
      }
    }

    /* messages swirl around...do local init then wait for responses */

    /*
     * store my own heap entry
     */
    seginfo_table[__state.mype].addr = great_big_heap;
    seginfo_table[__state.mype].size = __state.heapsize;

    /*
     * initialize my heap
     */
    __mem_init(seginfo_table[__state.mype].addr,
	       seginfo_table[__state.mype].size);

    {
      /* now wait on the AM replies */
      int got_all = __state.numpes - 2; /* 0-based AND don't count myself */
      do {
	__comms_pause();
      } while (seg_setup_replies_received <= got_all);
    }
  }

#endif /* HAVE_MANAGED_SEGMENTS */

  /* and make sure everyone is up-to-speed */
  __comms_barrier_all();

  /*
   * spit out the seginfo table (but check first that the loop is
   * warranted)
   */
  if (__trace_is_enabled(SHMEM_LOG_INIT)) {
    int pe;
    for (pe = 0; pe < __state.numpes; pe += 1) {
      __shmem_trace(SHMEM_LOG_INIT,
		    "cross-check: segment[%d] = { .addr = %p, .size = %ld }",
		    pe,
		    seginfo_table[pe].addr,
		    seginfo_table[pe].size
		    );
    }
  }
}

void
__symmetric_memory_finalize(void)
{
  __mem_finalize();
#if ! defined(HAVE_MANAGED_SEGMENTS)
  free(great_big_heap);
#endif /* HAVE_MANAGED_SEGMENTS */
}

/*
 * where the symmetric memory starts on the given PE
 */
void *
__symmetric_var_base(int pe)
{
  return seginfo_table[pe].addr;
}

/*
 * is the address in the managed symmetric area?
 */
int
__symmetric_var_in_range(void *addr, int pe)
{
  int retval;

  if (addr < seginfo_table[pe].addr) {
    retval = 0;
  }
  else if (addr > (seginfo_table[pe].addr + seginfo_table[pe].size)) {
    retval = 0;
  }
  else {
    retval = 1;
  }

  return retval;
}

/*
 * translate my "dest" to corresponding address on PE "pe"
 */
void *
__symmetric_addr_lookup(void *dest, int pe)
{
  size_t offset;
  char *rdest;

  if (dest == NULL) {
    return NULL;
  }

  if (__comms_is_globalvar(dest)) {
    return dest;
  }

  /* short-circuit a lookup on myself */
  if (__state.mype == pe) {
    rdest = dest;
  }
  else {
    offset = (char *) dest - (char *) __symmetric_var_base(__state.mype);
    rdest = (char *) __symmetric_var_base(pe) + offset;
  }

  if (__symmetric_var_in_range(rdest, pe)) {
    return (void *) rdest;
  }

  return NULL;
}

/*
 * check that the address is accessible to shmem on that PE
 *
 */

int
__comms_addr_accessible(void *addr, int pe)
{
  return (__symmetric_addr_lookup(addr, pe) != NULL);
}

/*
 * -- swap handlers ---------------------------------------------------------
 */
static gasnet_hsl_t swap_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t swap_bak_lock = GASNET_HSL_INITIALIZER;

/*
 * NB we make the cond/value "long long" throughout
 * to be used by smaller types as self-contained payload
 */

typedef struct {
  void *local_store;		/* sender saves here */
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	        /* addr of marker */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be swapped */
} swap_payload_t;

/*
 * called by remote PE to do the swap.  Store new value, send back old value
 */
static void
handler_swap_out(gasnet_token_t token,
		 void *buf, size_t bufsiz,
		 gasnet_handlerarg_t unused)
{
  long long old;
  swap_payload_t *pp = (swap_payload_t *) buf;

  gasnet_hsl_lock(& swap_out_lock);

  /* save and update */
  (void) memcpy(&old, pp->r_symm_addr, pp->nbytes);
  (void) memcpy(pp->r_symm_addr, &(pp->value), pp->nbytes);
  pp->value = old;

  gasnet_hsl_unlock(& swap_out_lock);

  /* return updated payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_SWAP_BAK, buf, bufsiz, unused);
}

/*
 * called by swap invoker when old value returned by remote PE
 */
static void
handler_swap_bak(gasnet_token_t token,
		 void *buf, size_t bufsiz,
		 gasnet_handlerarg_t unused)
{
  swap_payload_t *pp = (swap_payload_t *) buf;

  gasnet_hsl_lock(& swap_bak_lock);

  /* save returned value */
  (void) memcpy(pp->local_store, &(pp->value), pp->nbytes);

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock(& swap_bak_lock);
}

void
__comms_swap_request(void *target, void *value, size_t nbytes, int pe, void *retval)
{
  swap_payload_t *p = (swap_payload_t *) malloc(sizeof(*p));
  if (p == (swap_payload_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unable to allocate swap payload memory"
		  );
  }
  /* build payload to send */
  p->local_store = retval;
  p->r_symm_addr = __symmetric_addr_lookup(target, pe);
  p->nbytes = nbytes;
  p->value = *(long long *) value;
  p->completed = 0;
  p->completed_addr = &(p->completed);

  /* send and wait for ack */
  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_SWAP_OUT,
			  p, sizeof(*p),
			  0);
  WAIT_ON_COMPLETION(p->completed);

  free(p);
}

static gasnet_hsl_t cswap_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t cswap_bak_lock = GASNET_HSL_INITIALIZER;

typedef struct {
  void *local_store;		/* sender saves here */
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	        /* addr of marker */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be swapped */
  long long cond;		/* conditional value */
} cswap_payload_t;

/*
 * called by remote PE to do the swap.  Store new value if cond
 * matches, send back old value in either case
 */

#if 0
static void
handler_cswap_out(gasnet_token_t token,
		  void *buf, size_t bufsiz,
		  gasnet_handlerarg_t unused)
{
  long long old;
  cswap_payload_t *pp = (cswap_payload_t *) buf;

  gasnet_hsl_lock(& cswap_out_lock);

  /* save current target */
  old = *(long long *) pp->r_symm_addr;
  /* update value if cond matches */
  if ( *(long long *) pp->cond == *(long long *) pp->r_symm_addr) {
    *(long long *) pp->r_symm_addr = pp->value;
  }
  /* return value */
  pp->value = old;

  gasnet_hsl_unlock(& cswap_out_lock);

  /* return updated payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_CSWAP_BAK, buf, bufsiz, unused);
}
#endif

static void
handler_cswap_out(gasnet_token_t token,
		  void *buf, size_t bufsiz,
		  gasnet_handlerarg_t unused)
{
  void *old;
  cswap_payload_t *pp = (cswap_payload_t *) buf;

  gasnet_hsl_lock(& cswap_out_lock);

  old = malloc(pp->nbytes);

  /* save current target */
  memcpy(old, pp->r_symm_addr, pp->nbytes);

  /* update value if cond matches */
  if (memcmp(&(pp->cond), pp->r_symm_addr, pp->nbytes) == 0) {
    memcpy(pp->r_symm_addr, &(pp->value), pp->nbytes);
  }
  /* return value */
  memcpy(&(pp->value), old, pp->nbytes);

  free(old);

  gasnet_hsl_unlock(& cswap_out_lock);

  /* return updated payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_CSWAP_BAK, buf, bufsiz, unused);
}

/*
 * called by swap invoker when old value returned by remote PE
 * (same as swap_bak for now)
 */
static void
handler_cswap_bak(gasnet_token_t token,
		  void *buf, size_t bufsiz,
		  gasnet_handlerarg_t unused)
{
  cswap_payload_t *pp = (cswap_payload_t *) buf;

  gasnet_hsl_lock(& cswap_bak_lock);

  /* save returned value */
  (void) memcpy(pp->local_store, &(pp->value), pp->nbytes);

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock(& cswap_bak_lock);
}

void
__comms_cswap_request(void *target, void *cond, void *value, size_t nbytes,
		      int pe,
		      void *retval)
{
  cswap_payload_t *cp = (cswap_payload_t *) malloc(sizeof(*cp));
  if (cp == (cswap_payload_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unable to allocate conditional swap payload memory"
		  );
  }
  /* build payload to send */
  cp->local_store = retval;
  cp->r_symm_addr = __symmetric_addr_lookup(target, pe);
  cp->nbytes = nbytes;
  cp->value = cp->cond = 0LL;
  memcpy(&(cp->value), value, nbytes);
  memcpy(&(cp->cond), cond, nbytes);
  cp->completed = 0;
  cp->completed_addr = &(cp->completed);

  /* send and wait for ack */
  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_CSWAP_OUT,
			  cp, sizeof(*cp),
			  0);
  WAIT_ON_COMPLETION(cp->completed);

  free(cp);
}

/*
 * fetch/add
 */

typedef struct {
  void *local_store;		/* sender saves here */
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	        /* addr of marker */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be added & then return old */
} fadd_payload_t;

static gasnet_hsl_t fadd_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t fadd_bak_lock = GASNET_HSL_INITIALIZER;

/*
 * called by remote PE to do the fetch and add.  Store new value, send
 * back old value
 */
static void
handler_fadd_out(gasnet_token_t token,
		 void *buf, size_t bufsiz,
		 gasnet_handlerarg_t unused)
{
  long long old = 0;
  long long plus = 0;
  fadd_payload_t *pp = (fadd_payload_t *) buf;

  gasnet_hsl_lock(& fadd_out_lock);

  /* save and update */
  (void) memcpy(&old, pp->r_symm_addr, pp->nbytes);
  plus = old + pp->value;
  (void) memcpy(pp->r_symm_addr, &plus, pp->nbytes);
  pp->value = old;

  gasnet_hsl_unlock(& fadd_out_lock);

  /* return updated payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_FADD_BAK, buf, bufsiz, unused);
}

/*
 * called by fadd invoker when old value returned by remote PE
 */
static void
handler_fadd_bak(gasnet_token_t token,
		 void *buf, size_t bufsiz,
		 gasnet_handlerarg_t unused)
{
  fadd_payload_t *pp = (fadd_payload_t *) buf;

  gasnet_hsl_lock(& fadd_bak_lock);

  /* save returned value */
  (void) memcpy(pp->local_store, &(pp->value), pp->nbytes);

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock(& fadd_bak_lock);
}

void
__comms_fadd_request(void *target, void *value, size_t nbytes, int pe, void *retval)
{
  fadd_payload_t *p = (fadd_payload_t *) malloc(sizeof(*p));
  if (p == (fadd_payload_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unable to allocate fetch-and-add payload memory"
		  );
  }
  /* build payload to send */
  p->local_store = retval;
  p->r_symm_addr = __symmetric_addr_lookup(target, pe);
  p->nbytes = nbytes;
  p->value = *(long long *) value;
  p->completed = 0;
  p->completed_addr = &(p->completed);

  /* send and wait for ack */
  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_FADD_OUT,
			  p, sizeof(*p),
			  0);
  WAIT_ON_COMPLETION(p->completed);

  free(p);
}

/*
 * fetch/increment
 */

typedef struct {
  void *local_store;		/* sender saves here */
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	        /* addr of marker */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be returned */
} finc_payload_t;

static gasnet_hsl_t finc_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t finc_bak_lock = GASNET_HSL_INITIALIZER;

/*
 * called by remote PE to do the fetch and increment.  Store new
 * value, send back old value
 */
static void
handler_finc_out(gasnet_token_t token,
		 void *buf, size_t bufsiz,
		 gasnet_handlerarg_t unused)
{
  long long old = 0;
  long long plus = 1;
  finc_payload_t *pp = (finc_payload_t *) buf;

  gasnet_hsl_lock(& finc_out_lock);

  /* save and update */
  (void) memcpy(&old, pp->r_symm_addr, pp->nbytes);
  plus = old + 1;
  (void) memcpy(pp->r_symm_addr, &plus, pp->nbytes);
  pp->value = old;

  gasnet_hsl_unlock(& finc_out_lock);

  /* return updated payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_FINC_BAK, buf, bufsiz, unused);
}

/*
 * called by finc invoker when old value returned by remote PE
 */
static void
handler_finc_bak(gasnet_token_t token,
		 void *buf, size_t bufsiz,
		 gasnet_handlerarg_t unused)
{
  finc_payload_t *pp = (finc_payload_t *) buf;

  gasnet_hsl_lock(& finc_bak_lock);

  /* save returned value */
  (void) memcpy(pp->local_store, &(pp->value), pp->nbytes);

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock(& finc_bak_lock);
}

void
__comms_finc_request(void *target, size_t nbytes, int pe, void *retval)
{
  finc_payload_t *p = (finc_payload_t *) malloc(sizeof(*p));
  if (p == (finc_payload_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unable to allocate fetch-and-increment payload memory"
		  );
  }
  /* build payload to send */
  p->local_store = retval;
  p->r_symm_addr = __symmetric_addr_lookup(target, pe);
  p->nbytes = nbytes;
  p->completed = 0;
  p->completed_addr = &(p->completed);

  /* send and wait for ack */
  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_FINC_OUT,
			  p, sizeof(*p),
			  0);
  WAIT_ON_COMPLETION(p->completed);

  free(p);
}

/*
 * remote add
 */

typedef struct {
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	        /* addr of marker */
  size_t nbytes;		/* how big the value is */
  long long value;		/* value to be returned */
} add_payload_t;

static gasnet_hsl_t add_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t add_bak_lock = GASNET_HSL_INITIALIZER;

/*
 * called by remote PE to do the remote add.
 */
static void
handler_add_out(gasnet_token_t token,
		void *buf, size_t bufsiz,
		gasnet_handlerarg_t unused)
{
  long long old = 0;
  long long plus = 0;
  add_payload_t *pp = (add_payload_t *) buf;

  gasnet_hsl_lock(& add_out_lock);

  /* save and update */
  (void) memcpy(&old, pp->r_symm_addr, pp->nbytes);
  plus = old + pp->value;
  (void) memcpy(pp->r_symm_addr, &plus, pp->nbytes);

  gasnet_hsl_unlock(& add_out_lock);

  /* return updated payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_ADD_BAK, buf, bufsiz, unused);
}

/*
 * called by remote add invoker when store done
 */
static void
handler_add_bak(gasnet_token_t token,
		void *buf, size_t bufsiz,
		gasnet_handlerarg_t unused)
{
  add_payload_t *pp = (add_payload_t *) buf;

  gasnet_hsl_lock(& add_bak_lock);

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock(& add_bak_lock);
}

void
__comms_add_request(void *target, void *value, size_t nbytes, int pe)
{
  add_payload_t *p = (add_payload_t *) malloc(sizeof(*p));
  if (p == (add_payload_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unable to allocate remote add payload memory"
		  );
  }
  /* build payload to send */
  p->r_symm_addr = __symmetric_addr_lookup(target, pe);
  p->nbytes = nbytes;
  p->value = *(long long *) value;
  p->completed = 0;
  p->completed_addr = &(p->completed);

  /* send and wait for ack */
  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_ADD_OUT,
			  p, sizeof(*p),
			  0);
  WAIT_ON_COMPLETION(p->completed);

  free(p);
}

/*
 * remote increment
 */

typedef struct {
  void *r_symm_addr;		/* recipient symmetric var */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	        /* addr of marker */
  size_t nbytes;		/* how big the value is */
} inc_payload_t;

static gasnet_hsl_t inc_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t inc_bak_lock = GASNET_HSL_INITIALIZER;

/*
 * called by remote PE to do the remote increment
 */
static void
handler_inc_out(gasnet_token_t token,
		void *buf, size_t bufsiz,
		gasnet_handlerarg_t unused)
{
  long long old = 0;
  long long plus = 1;
  inc_payload_t *pp = (inc_payload_t *) buf;

  gasnet_hsl_lock(& inc_out_lock);

  /* save and update */
  (void) memcpy(&old, pp->r_symm_addr, pp->nbytes);
  plus = old + 1;
  (void) memcpy(pp->r_symm_addr, &plus, pp->nbytes);

  gasnet_hsl_unlock(& inc_out_lock);

  /* return updated payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_INC_BAK, buf, bufsiz, unused);
}

/*
 * called by remote increment invoker when store done
 */
static void
handler_inc_bak(gasnet_token_t token,
		void *buf, size_t bufsiz,
		gasnet_handlerarg_t unused)
{
  inc_payload_t *pp = (inc_payload_t *) buf;

  gasnet_hsl_lock(& inc_bak_lock);

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock(& inc_bak_lock);
}

void
__comms_inc_request(void *target, size_t nbytes, int pe)
{
  inc_payload_t *p = (inc_payload_t *) malloc(sizeof(*p));
  if (p == (inc_payload_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unable to allocate remote increment payload memory"
		  );
  }
  /* build payload to send */
  p->r_symm_addr = __symmetric_addr_lookup(target, pe);
  p->nbytes = nbytes;
  p->completed = 0;
  p->completed_addr = &(p->completed);

  /* send and wait for ack */
  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_INC_OUT,
			  p, sizeof(*p),
			  0);
  WAIT_ON_COMPLETION(p->completed);

  free(p);
}



/*
 * ---------------------------------------------------------------------------
 *
 * Handlers for pinging for shmem_pe_accessible
 *
 */

typedef struct {
  pe_status_t remote_pe_status;	/* health of remote PE */
  volatile int completed;	/* transaction end marker */
  volatile int *completed_addr;	        /* addr of marker */
} ping_payload_t;

static int pe_acked = 1;

static jmp_buf jb;

static void
ping_timeout_handler(int signum)
{
  pe_acked = 0;

  longjmp(jb, 1);
}

static gasnet_hsl_t ping_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t ping_bak_lock = GASNET_HSL_INITIALIZER;

/*
 * called by remote PE when (if) it gets the ping
 */
static void
handler_ping_out(gasnet_token_t token,
		 void *buf, size_t bufsiz,
		 gasnet_handlerarg_t unused)
{
  ping_payload_t *pp = (ping_payload_t *) buf;

  gasnet_hsl_lock(& ping_out_lock);

  pp->remote_pe_status = PE_RUNNING;

  /* sleep(__state.mype); */

  gasnet_hsl_unlock(& ping_out_lock);

  /* return ack'ed payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_PING_BAK, buf, bufsiz, unused);
}

/*
 * called by sender PE when (if) remote PE ack's the ping
 */
static void
handler_ping_bak(gasnet_token_t token,
		 void *buf, size_t bufsiz,
		 gasnet_handlerarg_t unused)
{
  ping_payload_t *pp = (ping_payload_t *) buf;
  
  gasnet_hsl_lock(& ping_bak_lock);

  pe_acked = pe_acked && (pp->remote_pe_status == PE_RUNNING);

  /* done it */
  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock(& ping_bak_lock);
}

int
__comms_ping_request(int pe)
{
  sighandler_t sig;
  int sj_status;
  ping_payload_t *p = (ping_payload_t *) malloc(sizeof(*p));
  if (p == (ping_payload_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unable to allocate remote accessibility payload memory"
		  );
  }
  /* build payload to send */
  p->completed = 0;
  p->completed_addr = &(p->completed);
  p->remote_pe_status = PE_UNKNOWN;

  /* now the ping is ponged, or we timeout waiting... */
  sig = signal(SIGALRM, ping_timeout_handler);
  if (sig == SIG_ERR) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: registration of ping timeout handler failed"
		  );
    /* NOT REACHED */
  }

  /* hope for the best */
  pe_acked = 1;

  __ping_set_alarm();

  sj_status = setjmp(jb);

  /* only ping if we're coming through the first time */
  if (sj_status == 0) {
    /* send and wait for ack */
    gasnet_AMRequestMedium1(pe, GASNET_HANDLER_PING_OUT,
			    p, sizeof(*p),
			    0);

    WAIT_ON_COMPLETION(p->completed);
  }

  __ping_clear_alarm();

  sig = signal(SIGALRM, sig);
  if (sig == SIG_ERR) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: release of ping timeout handler failed"
		  );
    /* NOT REACHED */
  }

  free(p);

  return pe_acked;
}


/*
 * ---------------------------------------------------------------------------
 */

#if defined(HAVE_MANAGED_SEGMENTS)

/*
 * global variable put/get handlers (for non-everything cases):
 *
 * 1. sender AMs remote with address of variable to write into, total size to write, and
 * current offset (like seek), then the data itself.
 *
 * 2. remote acks
 *
 * (repeat as needed if data size > max request length, updating offset)
 *
 * 3. done?
 *
 * HOW DO WE HANDLE WAITING FOR OUTSTANDING OPS IN THIS FRAMEWORK FOR BARRIERS etc.?
 * INSERT MEMBAR?
 *
 */

static gasnet_hsl_t globalvar_out_lock = GASNET_HSL_INITIALIZER;
static gasnet_hsl_t globalvar_bak_lock = GASNET_HSL_INITIALIZER;

typedef struct {
  void *store_addr;		/* address of global var to be written to on remote PE */
  long offset;		        /* where we are in the write process */
  long bytes_left;		/* how much data remains to be written */
  void *send_data;		/* the actual data to be sent */
  volatile int completed;	/* completion marker */
  volatile int *completed_addr;		/* addr of completion marker */
} globalvar_payload_t;


/* TODO: these are all stubs, still in thinking phase */

/*
 * called by remote PE to grab and write to its variable
 */
static void
handler_globalvar_out(gasnet_token_t token,
		      void *buf, size_t bufsiz,
		      gasnet_handlerarg_t unused)
{
  globalvar_payload_t *pp = (globalvar_payload_t *) buf;

  gasnet_hsl_lock(& globalvar_out_lock);

  gasnet_hsl_unlock(& globalvar_out_lock);

  /* return the updated payload */
  gasnet_AMReplyMedium1(token, GASNET_HANDLER_GLOBALVAR_BAK, buf, bufsiz, unused);
}

static void
handler_globalvar_bak(gasnet_token_t token,
		      void *buf, size_t bufsiz,
		      gasnet_handlerarg_t unused)
{
  globalvar_payload_t *pp = (globalvar_payload_t *) buf;

  gasnet_hsl_lock(& globalvar_bak_lock);

  *(pp->completed_addr) = 1;

  gasnet_hsl_unlock(& globalvar_bak_lock);
}

void
__comms_globalvar_translation(void *target, long value, int pe)
{
  globalvar_payload_t *p = (globalvar_payload_t *) malloc(sizeof(*p));

  if (p == (globalvar_payload_t *) NULL) {
    __shmem_trace(SHMEM_LOG_FATAL,
		  "internal error: unable to allocate heap variable payload memory"
		  );
  }

  p->completed = 0;
  p->completed_addr = &(p->completed); /* this one, not the copy */

  gasnet_AMRequestMedium1(pe, GASNET_HANDLER_GLOBALVAR_OUT,
			  p, sizeof(*p),
			  0);

  WAIT_ON_COMPLETION(p->completed);

  free(p);
}

#endif /* HAVE_MANAGED_SEGMENTS */
