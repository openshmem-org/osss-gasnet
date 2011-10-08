
SHELL = /bin/sh

SHMEM_CFLAGS=

GCC=gcc

CC = oshcc


# Please set these variables. They are used in execution time.
# #
# # the run command consists
# # > $RUNCMD $RUNOPT $NCOREOPT $NCORE[...] $NPROCOPT $NPROC[...]  executable

RUNCMD=oshrun
RUNOPT=
NCOREOPT= 
NCORE=
NPROCOPT=-np
NPROC=2

C_TESTS        = $(wildcard *.c)

C_EXES         = $(C_TESTS:.c=.x)

EXES           = $(C_EXES)

.PHONY: clean all default

all default:    $(EXES)

.SUFFIXES: .x

%.x:    %.c
	        $(CC) $(SHMEM_CFLAGS) -o $@ $^



run:
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./test_shmem_accessible.x
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./test_shmem_atomics.x
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./test_shmem_barrier.x
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./test_shmem_broadcast.x
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./test_shmem_collects.x
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./test_shmem_get_globals.x
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./test_shmem_get_shmalloc.x
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./test_shmem_lock.x
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./test_shmem_put_globals.x
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./test_shmem_put_shmalloc.x
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./test_shmem_reduction.x
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./test_shmem_synchronization.x


clean:
	        rm -f $(EXES)


