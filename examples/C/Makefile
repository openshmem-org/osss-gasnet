SHMEM_CC = oshcc

TESTS = hello.c multi-init.c \
		randput.c rotput.c rotget.c \
		badput.c badget.c arrput.c \
		testwait.c testwaituntil.c \
		bcast1.c bcast2.c \
		fcollect.c \
		dip.c fip.c iip.c lip.c \
		sping.c

EXES = $(TESTS:.c=.x)

.PHONY: clean all default

all default:	$(EXES)

.SUFFIXES: .x

%.x:	%.c
	$(SHMEM_CC) -g -o $@ $^

clean:
	rm -f $(EXES)
