SHMEM_CC = oshcc

TESTS = $(wildcard *.c)

EXES = $(TESTS:.c=.x)

.PHONY: clean all default

all default:	$(EXES)

.SUFFIXES: .x

%.x:	%.c
	$(SHMEM_CC) -g -o $@ $^

clean:
	rm -f $(EXES)
