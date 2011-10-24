
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
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./put_performance.x
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./get_performance.x
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./barrier_performance.x
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./broadcast32_performance.x
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./collect32_performance.x
	 $(RUNCMD) $(RUNOPT) $(NPROCOPT) $(NPROC) ./fcollect32_performance.x


clean:
	        rm -f $(EXES)


