SHMEM_CC = oshcc

TESTS = \
		hello.c hellofrom.c \
		multi-init.c lock.c \
		randput.c rotput.c rotget.c \
		badput.c badget.c arrput.c \
		bad-malloc.c \
		testwait.c testwaituntil.c \
		bcast1.c bcast2.c \
		collect32.c collect64.c \
		fcollect32.c fcollect64.c \
		dip.c fip.c iip.c lip.c \
		iput.c iget.c \
		add.c fadd.c \
		sping.c swap.c version.c \
		reduce-sum.c reduce-max.c

EXES = $(TESTS:.c=.x)

.PHONY: clean all default

all default:	$(EXES)

.SUFFIXES: .x

%.x:	%.c
	$(SHMEM_CC) -g -o $@ $^

clean:
	rm -f $(EXES)
