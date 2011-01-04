include ../make.include

# ---------------------------------------------------------

DLMALLOC_DIR     = ./dlmalloc
DLMALLOC_CFLAGS  = -I$(DLMALLOC_DIR)
DLMALLOC_OBJECT  = $(DLMALLOC_DIR)/dlmalloc.o

MALLOC_OBJECT    = $(DLMALLOC_OBJECT)

# ---------------------------------------------------------

CC               = gcc

CPPFLAGS        += -I../include
CPPFLAGS        += -MMD

CPPFLAGS        += -DHAVE_PSHMEM_SUPPORT

CFLAGS          += -fPIC
CFLAGS          += -std=c99

DEBUG_AND_OPT    = -ggdb -O3

WARNINGS         = -Wall
# WARNINGS        += -pedantic

COMMON_FLAGS     = $(CPPFLAGS) $(CFLAGS) $(DEBUG_AND_OPT) 

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
SHMEM_LIBRARY    = libopenshmem.so
SHMEM_HEADERS    = shmem.h shmem.fh
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
.PHONY: do-deps

# ---------------------------------------------------------

all default:	do-deps $(SHMEM_LIBRARY) wrappers

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

version.c:
	./set-version.sh $@

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
	@ $(REMOVE)  $(SHMEM_CC)
	@ echo "#!/bin/sh" >> $(SHMEM_CC)
	@ echo "#" >> $(SHMEM_CC)
	@ echo "# generated from OpenSHMEM src directory, don't edit installed version" >> $(SHMEM_CC)
	@ echo "#" >> $(SHMEM_CC)
	@ echo "SHMEM_LDFLAGS=\"-L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib\"" >> $(SHMEM_CC)
	@ echo "SHMEM_LIB=\"-lopenshmem\"" >> $(SHMEM_CC)
	@ echo "SHMEM_INCLUDE_DIR=\"$(PREFIX)/include\"" >> $(SHMEM_CC)
	@ echo exec $(GASNET_LD) -I\$$SHMEM_INCLUDE_DIR $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $$\@ \$$SHMEM_LDFLAGS \$$SHMEM_LIB $(GASNET_LDFLAGS) $(GASNET_LIBS) >> $(SHMEM_CC)

f-wrapper:
	@ $(REMOVE)  $(SHMEM_FC)
	@ echo "#!/bin/sh" >> $(SHMEM_FC)
	@ echo "#" >> $(SHMEM_FC)
	@ echo "# generated from OpenSHMEM src directory, don't edit installed version" >> $(SHMEM_FC)
	@ echo "#" >> $(SHMEM_FC)
	@ echo "SHMEM_LDFLAGS=\"-L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib\"" >> $(SHMEM_FC)
	@ echo "SHMEM_LIB=\"-lopenshmem\"" >> $(SHMEM_FC)
	@ echo "SHMEM_INCLUDE_DIR=\"$(PREFIX)/include\"" >> $(SHMEM_FC)
	@ echo exec $(GASNET_LD) -I\$$SHMEM_INCLUDE_DIR $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $$\@ \$$SHMEM_LDFLAGS \$$SHMEM_LIB $(GASNET_LDFLAGS) $(GASNET_LIBS) >> $(SHMEM_FC)
	TMP=`mktemp`; cp $(SHMEM_FC) $$TMP; sed -e 's%bin/mpicc%bin/mpif90%' $$TMP > $(SHMEM_FC) ; $(REMOVE) $$TMP

# ---------------------------------------------------------

tidy:
	$(REMOVE) $(SHMEM_OBJECTS)
	$(REMOVE) $(SHMEM_DEPFILES)

clean:	tidy
	$(MAKE) -C $(DLMALLOC_DIR) $@
	$(REMOVE) $(SHMEM_LIBRARY) $(SHMEM_CC)
	$(REMOVE) Makedeps

do-deps: $(SHMEM_OBJECTS)
	- cat $(SHMEM_DEPFILES) > Makedeps

# ---------------------------------------------------------

-include Makedeps
