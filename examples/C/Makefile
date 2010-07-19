SHMEM_CC = oshcc

TESTS = hello.c multi-init.c \
		randput.c rotput.c rotget.c \
		badput.c badget.c arrput.c \
		testwait.c \
		bcast1.c bcast2.c \
		fcollect.c \
		sping.c

EXES = $(TESTS:.c=.x)

.PHONY: clean all default

all default:	$(EXES)

.SUFFIXES: .x

%.x:	%.c
	$(SHMEM_CC) -o $@ $^

clean:
	rm -f $(EXES)
