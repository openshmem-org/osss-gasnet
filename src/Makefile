include ../make.include

# ---------------------------------------------------------

DLMALLOC_DIR     = ./dlmalloc
DLMALLOC_CFLAGS  = -I$(DLMALLOC_DIR)
DLMALLOC_OBJECT  = $(DLMALLOC_DIR)/dlmalloc.o

MALLOC_OBJECT    = $(DLMALLOC_OBJECT)

# ---------------------------------------------------------

#
# this is where the C compiler gets chosen along with the various
# flags that do PIC, dependency, debug/optimize and C99 compliance
#

# Sun/Oracle Studio: state = testing
#CC               = suncc
#CPPFLAGS        += -xMMD
#CFLAGS          += -PIC
#CFLAGS          += -xc99=all
#DEBUG_AND_OPT    = -ggdb -xO3
#WARNINGS         =
# WARNINGS        += -pedantic

# GNU C: requires >= 4.1.0 version
CC               = gcc
CPPFLAGS        += -MMD
CFLAGS          += -fPIC
CFLAGS          += -std=c99
DEBUG_AND_OPT    = -ggdb -O3
WARNINGS         = -Wall
# WARNINGS        += -pedantic

# ---------------------------------------------------------

#
# compiler-independent options
#

CPPFLAGS        += -I../include
CPPFLAGS        += -DHAVE_PSHMEM_SUPPORT

COMMON_FLAGS     = $(CPPFLAGS) $(CFLAGS) $(DEBUG_AND_OPT) 

# ---------------------------------------------------------

# communication library dependent
SHMEM_SOURCES    =	comms.c
# internal
SHMEM_SOURCES   +=	env.c hooks.c state.c memalloc.c
# Fortran API
SHMEM_SOURCES   +=	fortran-mem.c fortran.c
# API
SHMEM_SOURCES   +=	accessible.c ptr.c cache.c \
			atomic.c fence.c \
			barrier.c broadcast.c collect.c fcollect.c \
			lock.c \
			putget.c query.c reduce-op.c \
			strided.c symmem.c updown.c waituntil.c warn.c \
			version.c

SHMEM_OBJECTS    = $(SHMEM_SOURCES:.c=.o)
SHMEM_DEPFILES   = $(SHMEM_SOURCES:.c=.d)
SHMEM_LIBNAME    = openshmem
SHMEM_LIBRARY    = lib$(SHMEM_LIBNAME).so
SHMEM_LIBS       = -l$(SHMEM_LIBNAME)
SHMEM_HEADERS    = shmem.h shmem.fh
SHMEM_INC_DIR    = $(PREFIX)/include
SHMEM_LDFLAGS    = -L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib

SHMEM_WRAPPER    = osh-compiler.in
SHMEM_CC         = oshcc
SHMEM_FC         = oshfort
SHMEM_RUN        = oshrun

INSTALL          = install -c
REMOVE           = rm -f

# ---------------------------------------------------------

.PHONY: c-wrapper f-wrapper wrappers
.PHONY: install install-lib install-bin install-include
.PHONY: uninstall uninstall-lib uninstall-bin uninstall-include
.PHONY: tidy clean
.PHONY: set-version do-deps

# ---------------------------------------------------------

all default:	set-version do-deps $(SHMEM_LIBRARY) wrappers

install:	install-lib install-bin install-include

install-lib:	all
	mkdir -p $(PREFIX)/lib
	$(INSTALL) $(SHMEM_LIBRARY) $(PREFIX)/lib

install-bin:	all
	mkdir -p $(PREFIX)/bin
	$(INSTALL) -m 755 $(SHMEM_CC) $(SHMEM_FC) $(PREFIX)/bin
	$(INSTALL) -m 755 $(SHMEM_RUN) $(PREFIX)/bin

install-include:	all
	mkdir -p $(PREFIX)/include/mpp
	$(INSTALL) -m 644 $(SHMEM_HEADERS) $(PREFIX)/include
	(cd $(PREFIX)/include/mpp; for f in $(SHMEM_HEADERS); do $(REMOVE) $$f; ln -s ../$$f; done)

# we may be sharing the install area with other packages, so don't
# blindly zap directories when uninstalling, allow them to remain
# there gracefully.

uninstall:	uninstall-lib uninstall-bin uninstall-include
	- rmdir $(PREFIX)

uninstall-lib:
	$(REMOVE) $(PREFIX)/lib/$(SHMEM_LIBRARY)
	- rmdir $(PREFIX)/lib

uninstall-bin:
	$(REMOVE) $(PREFIX)/bin/$(SHMEM_CC)
	$(REMOVE) $(PREFIX)/bin/$(SHMEM_FC)
	$(REMOVE) $(PREFIX)/bin/$(SHMEM_RUN)
	- rmdir $(PREFIX)/bin

uninstall-include:
	for h in $(SHMEM_HEADERS); do $(REMOVE) $(PREFIX)/include/mpp/$$h; $(REMOVE) $(PREFIX)/include/$$h; done
	- rmdir $(PREFIX)/include/mpp
	- rmdir $(PREFIX)/include

# -- start dlmalloc ----------------------------------------

$(DLMALLOC_OBJECT):
	$(MAKE) -C $(DLMALLOC_DIR)

$(SHMEM_LIBRARY):	$(SHMEM_OBJECTS) $(MALLOC_OBJECT)
	$(GASNET_LD) -shared -Wl,-soname,$@ -o $@ $^

memalloc.o:	memalloc.c
	$(CC) $(COMMON_FLAGS) $(DLMALLOC_CFLAGS) -c -o $@ $<

# -- end dlmalloc ------------------------------------------

# -- start GASNet ------------------------------------------
# eventually, we'll migrate gasnet dependencies to a
# separate transport directory
#

comms.o:	comms.c
	$(CC) \
		$(GASNET_CPPFLAGS) $(GASNET_CFLAGS) \
		$(DLMALLOC_CFLAGS) \
		$(COMMON_FLAGS) -c -o $@ $<

# -- end GASNet --------------------------------------------

set-version:
	if [ -d .svn ]; then ./set-version.sh version.c; fi

# ---------------------------------------------------------

fortran.o:	fortran.c
	$(CC) $(COMMON_FLAGS) -DFORTRAN_SINGLE_UNDERSCORE -c -o $@ $<

fortran-mem.o:	fortran-mem.c
	$(CC) $(COMMON_FLAGS) -DFORTRAN_SINGLE_UNDERSCORE -c -o $@ $<

# ---------------------------------------------------------

%.o:	%.c
	$(CC) $(WARNINGS) $(COMMON_FLAGS) -c -o $@ $<

# ---------------------------------------------------------

wrappers:	c-wrapper f-wrapper

c-wrapper:
	sed -e "s%@SHMEM_LDFLAGS@%$(SHMEM_LDFLAGS)%" \
		-e "s%@SHMEM_LIBS@%$(SHMEM_LIBS)%" \
		-e "s%@SHMEM_INC_DIR@%$(SHMEM_INC_DIR)%" \
		-e "s%@GASNET_LDFLAGS@%$(GASNET_LDFLAGS)%" \
		-e "s%@GASNET_CPPFLAGS@%$(GASNET_CPPFLAGS)%" \
		-e "s%@GASNET_CFLAGS@%$(GASNET_CFLAGS)%" \
		-e "s%@GASNET_LIBS@%$(GASNET_LIBS)%" \
		-e "s%@GASNET_LD@%$(GASNET_LD)%" \
		< $(SHMEM_WRAPPER) > $(SHMEM_CC)

f-wrapper:
	sed -e "s%@SHMEM_LDFLAGS@%$(SHMEM_LDFLAGS)%" \
		-e "s%@SHMEM_LIBS@%$(SHMEM_LIBS)%" \
		-e "s%@SHMEM_INC_DIR@%$(SHMEM_INC_DIR)%" \
		-e "s%@GASNET_LDFLAGS@%$(GASNET_LDFLAGS)%" \
		-e "s%@GASNET_CPPFLAGS@%$(GASNET_CPPFLAGS)%" \
		-e "s%@GASNET_CFLAGS@%$(GASNET_CFLAGS)%" \
		-e "s%@GASNET_LIBS@%$(GASNET_LIBS)%" \
		-e "s%@GASNET_LD@%$(GASNET_LD)%" \
		-e 's%bin/mpicc%bin/mpif90%' \
		< $(SHMEM_WRAPPER) > $(SHMEM_FC)

# ---------------------------------------------------------

tidy:
	$(REMOVE) $(SHMEM_OBJECTS)
	$(REMOVE) $(SHMEM_DEPFILES)

clean:	tidy
	$(MAKE) -C $(DLMALLOC_DIR) $@
	$(REMOVE) $(SHMEM_LIBRARY) $(SHMEM_CC) $(SHMEM_FC)
	$(REMOVE) Makedeps

do-deps: $(SHMEM_OBJECTS)
	- cat $(SHMEM_DEPFILES) > Makedeps

# ---------------------------------------------------------

-include Makedeps
