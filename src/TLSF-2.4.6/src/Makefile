WARNINGS = -Wextra -Wall -Wwrite-strings -Wstrict-prototypes \
           -Wmissing-prototypes -Wno-long-long -Wstrict-aliasing=2

CFLAGS+= -g -O3 -I$(TLSF_PATH) $(WARNINGS) 
CFLAGS+= -DTLSF_USE_LOCKS=1
CFLAGS+= -DUSE_MMAP=0
CFLAGS+= -DUSE_SBRK=0
CFLAGS+= -fPIC

# CFLAGS+=-ftest-coverage -fprofile-arcs
CC=gcc

all: tlsf.o

tlsf.o: tlsf.h tlsf.c Makefile

clean:
	$(RM) -rf *.o *~ *.c.gcov *.gcda *.gcno

indent:
	indent -kr -l120 -br -ce -npsl -nut -i4 *.[hc]

