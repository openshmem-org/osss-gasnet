include ../make.include

# ---------------------------------------------------------

DLMALLOC_DIR     = ./dlmalloc
DLMALLOC_CFLAGS  = -I$(DLMALLOC_DIR)
DLMALLOC_OBJECT  = $(DLMALLOC_DIR)/dlmalloc.o

MALLOC_OBJECT    = $(DLMALLOC_OBJECT)

# ---------------------------------------------------------

CC               = gcc
CPPFLAGS        += -I../include
# CPPFLAGS        += -DSHMEM_COLLECT_STATS
CPPFLAGS        += -DHAVE_PSHMEM_SUPPORT
CPPFLAGS        += -MMD

CFLAGS          += -fPIC
 
#
# in case we ever want complex math support in C
#
# CFLAGS          += -std=c99

DEBUG_AND_OPT    = -g -O3

COMMON_FLAGS     = $(CPPFLAGS) $(CFLAGS) $(DEBUG_AND_OPT) 

SHMEM_SOURCES    = accessible.c atomic.c barrier.c broadcast.c \
			cache.c comms.c dispatch.c env.c fcollect.c \
			fence.c fortran-mem.c fortran.c hooks.c lock.c \
			memalloc.c ptr.c putget.c query.c reduce-and.c \
			reduce-common.c reduce-max.c state.c \
			strided.c symmem.c updown.c waituntil.c warn.c \
			version.c

SHMEM_OBJECTS    = $(SHMEM_SOURCES:.c=.o)
SHMEM_DEPFILES   = $(SHMEM_SOURCES:.c=.d)
SHMEM_LIBRARY    = libopenshmem.so
SHMEM_HEADERS    = shmem.h shmem.fh
SHMEM_CC         = oshcc
SHMEM_FC         = oshfort
SHMEM_RUN        = oshrun

# ---------------------------------------------------------

.PHONY: c-wrapper f-wrapper wrappers
.PHONY: install install-lib install-bin install-include
.PHONY: tidy clean
.PHONY: do-deps

all default:	do-deps $(SHMEM_LIBRARY) wrappers

install:	install-lib install-bin install-include

install-lib:	all
	mkdir -p $(PREFIX)/lib
	install -c  $(SHMEM_LIBRARY) $(PREFIX)/lib

install-bin:	all
	mkdir -p $(PREFIX)/bin
	install -c -m 755 $(SHMEM_CC) $(SHMEM_FC) $(PREFIX)/bin
	install -c -m 755 $(SHMEM_RUN) $(PREFIX)/bin

install-include:	all
	mkdir -p $(PREFIX)/include/mpp
	install -c  $(SHMEM_HEADERS) $(PREFIX)/include
	(cd $(PREFIX)/include/mpp; for f in $(SHMEM_HEADERS); do rm -f $$f; ln -s ../$$f; done)

# -- start dlmalloc ----------------------------------------

$(DLMALLOC_OBJECT):
	$(MAKE) -C $(DLMALLOC_DIR)

$(SHMEM_LIBRARY):	$(SHMEM_OBJECTS) $(MALLOC_OBJECT)
	$(GASNET_LD) -shared -Wl,-soname,$@ -o $@ $^

memalloc.o:	memalloc.c
	$(CC) $(COMMON_FLAGS) $(DLMALLOC_CFLAGS) -c -o $@ $<

putget.o:	putget.c
	$(CC) $(COMMON_FLAGS) $(DLMALLOC_CFLAGS) -c -o $@ $<

# -- end dlmalloc ------------------------------------------

# -- start GASNet ------------------------------------------
# eventually, we'll migrate gasnet dependencies to a
# separate transport directory
#

comms.o:	comms.c
	$(GASNET_CC) \
		$(GASNET_CPPFLAGS) $(GASNET_CFLAGS) \
		$(DLMALLOC_CFLAGS) \
		$(COMMON_FLAGS) -c -o $@ $<

# -- end GASNet --------------------------------------------

version.c:
	./set-version.sh

fortran.o:	fortran.c
	$(CC) $(COMMON_FLAGS) -DFORTRAN_SINGLE_UNDERSCORE -c -o $@ $<

fortran-mem.o:	fortran-mem.c
	$(CC) $(COMMON_FLAGS) -DFORTRAN_SINGLE_UNDERSCORE -c -o $@ $<

%.o:	%.c
	$(CC) $(COMMON_FLAGS) -c -o $@ $<

wrappers:	c-wrapper f-wrapper

c-wrapper:
	@ rm -f  $(SHMEM_CC)
	@ echo "#!/bin/sh" >> $(SHMEM_CC)
	@ echo "#" >> $(SHMEM_CC)
	@ echo "# generated from OpenSHMEM src directory, don't edit installed version" >> $(SHMEM_CC)
	@ echo "#" >> $(SHMEM_CC)
	@ echo "SHMEM_LDFLAGS=\"-L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib\"" >> $(SHMEM_CC)
	@ echo "SHMEM_LIB=\"-lopenshmem\"" >> $(SHMEM_CC)
	@ echo "SHMEM_INCLUDE_DIR=\"$(PREFIX)/include\"" >> $(SHMEM_CC)
	@ echo exec $(GASNET_LD) -I\$$SHMEM_INCLUDE_DIR $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $$\@ \$$SHMEM_LDFLAGS \$$SHMEM_LIB $(GASNET_LDFLAGS) $(GASNET_LIBS) >> $(SHMEM_CC)

f-wrapper:
	@ rm -f  $(SHMEM_FC)
	@ echo "#!/bin/sh" >> $(SHMEM_FC)
	@ echo "#" >> $(SHMEM_FC)
	@ echo "# generated from OpenSHMEM src directory, don't edit installed version" >> $(SHMEM_FC)
	@ echo "#" >> $(SHMEM_FC)
	@ echo "SHMEM_LDFLAGS=\"-L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib\"" >> $(SHMEM_FC)
	@ echo "SHMEM_LIB=\"-lopenshmem\"" >> $(SHMEM_FC)
	@ echo "SHMEM_INCLUDE_DIR=\"$(PREFIX)/include\"" >> $(SHMEM_FC)
	@ echo exec $(GASNET_LD) -I\$$SHMEM_INCLUDE_DIR $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $$\@ \$$SHMEM_LDFLAGS \$$SHMEM_LIB $(GASNET_LDFLAGS) $(GASNET_LIBS) >> $(SHMEM_FC)
	TMP=`mktemp`; cp $(SHMEM_FC) $$TMP; sed -e 's%bin/mpicc%bin/mpif90%' $$TMP > $(SHMEM_FC) ; rm -f $$TMP

tidy:
	rm -f $(SHMEM_OBJECTS)
	rm -f $(SHMEM_DEPFILES)

clean:	tidy
	$(MAKE) -C $(DLMALLOC_DIR) $@
	rm -f $(SHMEM_LIBRARY) $(SHMEM_CC)
	rm -f Makedeps

do-deps: $(SHMEM_OBJECTS)
	- cat $(SHMEM_DEPFILES) > Makedeps

-include Makedeps
