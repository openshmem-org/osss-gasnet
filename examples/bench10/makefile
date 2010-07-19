##########################################################################
#
#   Benchmark 10 (SHMEM) - makefile
#
#       make  -        will provide the basic benchmark
#
#       make clean   - will remove all *.o, *.V and core files
#
##########################################################################

CC	  = oshcc

OPTLEVEL  = -g

#
# define CRAY to get shmem_init()
#
CFLAGS    = $(OPTLEVEL)

SRCS      = $(wildcard *.c)
OBJS      = $(SRCS:.c=.o)

HEADERS   = b10defs.h

LDFLAGS   = $(OPTLEVEL)
LDLIBS    =

all:	bench10

bench10 : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f  *.o  *.V
