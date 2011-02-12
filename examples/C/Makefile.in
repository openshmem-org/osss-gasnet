SHMEM_CC = oshcc
SHMEM_CFLAGS = -ggdb -O3

TESTS = $(wildcard *.c)

EXES = $(TESTS:.c=.x)

.PHONY: clean all default

all default:	$(EXES)

.SUFFIXES: .x

%.x:	%.c
	$(SHMEM_CC) $(SHMEM_CFLAGS) -o $@ $^

clean:
	rm -f $(EXES)
