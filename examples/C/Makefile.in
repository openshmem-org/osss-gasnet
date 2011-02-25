SHMEM_CC       = oshcc
SHMEM_CFLAGS   = -ggdb -O0

C_TESTS        = $(wildcard *.c)

C_EXES         = $(C_TESTS:.c=.x)

EXES           = $(C_EXES)

.PHONY: clean all default

all default:	$(EXES)

.SUFFIXES: .x

%.x:	%.c
	$(SHMEM_CC) $(SHMEM_CFLAGS) -o $@ $^

clean:
	rm -f $(EXES)
