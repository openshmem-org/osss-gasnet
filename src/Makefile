PREFIX = $(HOME)/SHMEM

TLSF_DIR = ./TLSF/src
TLSF_CFLAGS = -I$(TLSF_DIR)
TLSF_OBJECT = $(TLSF_DIR)/tlsf.o

DLMALLOC_DIR = ./dlmalloc
DLMALLOC_OBJECT = $(DLMALLOC_DIR)/dlmalloc.o

# MALLOC_OBJECT = $(DLMALLOC_OBJECT)
MALLOC_OBJECT = $(TLSF_OBJECT)

GASNET = $(HOME)/local

include $(GASNET)/include/mpi-conduit/mpi-par.mak 

CPPFLAGS += -I../include
# CPPFLAGS += -DSHMEM_COLLECT_STATS
CPPFLAGS += -MMD

DEBUG_OR_OPT = -g

SHMEM_SOURCES  = $(wildcard *.c)
SHMEM_OBJECTS  = $(SHMEM_SOURCES:.c=.o)
SHMEM_DEPFILES = $(SHMEM_SOURCES:.c=.d)
SHMEM_LIBRARY  = libshmem.a
SHMEM_HEADERS  = shmem.h shmem.fh
SHMEM_CC       = oshcc
SHMEM_FC       = oshfort
SHMEM_RUN      = oshrun

# ---------------------------------------------------------

.PHONY: c-wrapper f-wrapper
.PHONY: tlsf dlmalloc
.PHONY: tidy clean wrappers
.PHONY: do-deps

all default:	do-deps tlsf $(SHMEM_LIBRARY) wrappers

install:	all
	mkdir -p $(PREFIX)/lib
	mkdir -p $(PREFIX)/include
	mkdir -p $(PREFIX)/bin
	install -c  $(SHMEM_LIBRARY) $(PREFIX)/lib
	install -c  $(SHMEM_HEADERS) $(PREFIX)/include
	install -c -m 755 $(SHMEM_CC) $(SHMEM_FC) $(PREFIX)/bin
	install -c -m 755 $(SHMEM_RUN) $(PREFIX)/bin

tlsf:
	$(MAKE) -C $(TLSF_DIR)

$(DLMALLOC_OBJECT):
	$(MAKE) -C $(DLMALLOC_DIR)

$(SHMEM_LIBRARY):	$(SHMEM_OBJECTS) $(MALLOC_OBJECT)
	rm -f $@
	ar cru $@ $^
	ranlib $@

#
# eventually, we'll migrate gasnet dependencies to a
# separate transport directory
#
.c.o:
	$(GASNET_CC) \
		$(CPPFLAGS) $(CFLAGS) \
		$(GASNET_CPPFLAGS) $(GASNET_CFLAGS) \
		$(TLSF_CFLAGS) \
		$(DEBUG_OR_OPT) -c -o $@ $<

wrappers:	c-wrapper f-wrapper

c-wrapper:
	@ rm -f  $(SHMEM_CC)
	@ echo "#!/bin/sh" >> $(SHMEM_CC)
	@ echo "#" >> $(SHMEM_CC)
	@ echo "# generated from OpenSHMEM src directory, don't edit installed version" >> $(SHMEM_CC)
	@ echo "#" >> $(SHMEM_CC)
	@ echo "SHMEM_LDFLAGS=\"-L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib\"" >> $(SHMEM_CC)
	@ echo "SHMEM_LIB=\"-lshmem\"" >> $(SHMEM_CC)
	@ echo "SHMEM_INCLUDE_DIR=\"$(PREFIX)/include\"" >> $(SHMEM_CC)
	@ echo exec $(GASNET_LD) -I\$$SHMEM_INCLUDE_DIR $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $$\@ \$$SHMEM_LDFLAGS \$$SHMEM_LIB $(GASNET_LDFLAGS) $(GASNET_LIBS) >> $(SHMEM_CC)

f-wrapper:
	@ rm -f  $(SHMEM_FC)
	@ echo "#!/bin/sh" >> $(SHMEM_FC)
	@ echo "#" >> $(SHMEM_FC)
	@ echo "# generated from OpenSHMEM src directory, don't edit installed version" >> $(SHMEM_FC)
	@ echo "#" >> $(SHMEM_FC)
	@ echo "SHMEM_LDFLAGS=\"-L$(PREFIX)/lib -Wl,-rpath,$(PREFIX)/lib\"" >> $(SHMEM_FC)
	@ echo "SHMEM_LIB=\"-lshmem\"" >> $(SHMEM_FC)
	@ echo "SHMEM_INCLUDE_DIR=\"$(PREFIX)/include\"" >> $(SHMEM_FC)
	@ echo exec $(GASNET_LD) -I\$$SHMEM_INCLUDE_DIR $(GASNET_CPPFLAGS) $(GASNET_CFLAGS) $$\@ \$$SHMEM_LDFLAGS \$$SHMEM_LIB $(GASNET_LDFLAGS) $(GASNET_LIBS) >> $(SHMEM_FC)
	TMP=`mktemp`; cp $(SHMEM_FC) $$TMP; sed -e 's%bin/mpicc%bin/mpif90%' $$TMP > $(SHMEM_FC) ; rm -f $$TMP

tidy:
	rm -f $(SHMEM_OBJECTS)
	rm -f $(SHMEM_DEPFILES)

clean:	tidy
	$(MAKE) -C $(TLSF_DIR) $@
	$(MAKE) -C $(DLMALLOC_DIR) $@
	rm -f $(SHMEM_LIBRARY) $(SHMEM_CC)
	rm -f Makedeps

do-deps: $(SHMEM_OBJECTS)
	cat $(SHMEM_DEPFILES) > Makedeps

-include Makedeps
