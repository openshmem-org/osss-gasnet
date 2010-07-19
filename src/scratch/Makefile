PREFIX = $(HOME)/SHMEM

TLSF_DIR = ./TLSF/src
TLSF_CFLAGS = -I$(TLSF_DIR)
TLSF_OBJECT = $(TLSF_DIR)/tlsf.o

DLMALLOC_DIR = ./dlmalloc
DLMALLOC_OBJECT = $(DLMALLOC_DIR)/dlmalloc.o

GASNET = $(HOME)/local

include $(GASNET)/include/mpi-conduit/mpi-par.mak 

CPPFLAGS += -I ../include
# CPPFLAGS += -DSHMEM_COLLECT_STATS

CFLAGS += -fPIC

DEBUG_OR_OPT = -O3

SHMEM_SOURCES = $(wildcard *.c)
SHMEM_OBJECTS = $(SHMEM_SOURCES:.c=.o)

SHMEM_LIBRARY = libshmem.so
SHMEM_HEADERS = shmem.h shmem.fh

SHMEM_CC = oshcc
SHMEM_FC = oshfort
SHMEM_RUN = oshrun

# ---------------------------------------------------------

all default:	tlsf $(SHMEM_LIBRARY) wrappers

install:	all
	install -c  $(SHMEM_LIBRARY) ../lib/
	install -c  $(SHMEM_HEADERS) ../include/
	install -c -m 755 $(SHMEM_CC) $(SHMEM_FC) ../bin/
	install -c -m 755 $(SHMEM_RUN) ../bin/

tlsf:
	$(MAKE) -C $(TLSF_DIR)

$(SHMEM_LIBRARY):	$(SHMEM_OBJECTS) $(TLSF_OBJECT)
	rm -f $@
	$(GASNET_LD) --shared -Wl,-soname,$@ -o $@ $^

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

.PHONY: tlsf tidy clean wrappers

tidy:
	rm -f $(SHMEM_OBJECTS)

clean:	tidy
	$(MAKE) -C $(TLSF_DIR) $@
	rm -f $(SHMEM_LIBRARY) $(SHMEM_CC)

.PHONY: c-wrapper f-wrapper

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
