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

# for Alpha Sierra Cluster
#CFLAGS    = -fast -arch host -tune host
# for Cray X1
CFLAGS    = -O3 -DCRAY -I.

CFLAGS   += -DDEBUG=1
OBJS      = m10.o p10.o c10.o r10.o util.o
HEADERS   = b10defs.h

# If you get an error about the following library missing
# then comment the following line
#LIBS      = -lshmem

# for Alpha Sierra Cluster
#LDFLAGS   = -lc -lm -lelan -lshmem
# LDFLAGS   = -lc -lm -lsma
all:	bench10


m10.o: m10.c $(HEADERS)
	$(CC) $(CFLAGS) -c m10.c

p10.o: p10.c $(HEADERS)
	$(CC) $(CFLAGS) -c p10.c

c10.o: c10.c $(HEADERS)
	$(CC) $(CFLAGS) -c c10.c

r10.o: r10.c $(HEADERS)
	$(CC) $(CFLAGS) -c r10.c

util.o: util.c $(HEADERS)
	$(CC) $(CFLAGS) -c util.c

bench10 : $(OBJS) makefile 
	$(CC) $(LDFLAGS)  -o bench10 $(OBJS) -lm

clean:
	rm -f  *.o  *.V
