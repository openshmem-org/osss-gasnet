CC               = gcc
CFLAGS           = -ggdb -O3
CPPFLAGS         = 
LD               = gcc
LDFLAGS          = 

CFLAGS          += -fPIC

.PHONY: all default clean

all default:	dlmalloc.o

dlmalloc.o:	dlmalloc.c
	$(CC) -c -DONLY_MSPACES=1 -DUSE_LOCKS=1 $(CFLAGS) $^

clean:
	rm -f dlmalloc.o
