##########################################################################
#
#   Benchmark 6 - makefile
#
#	Shmem version on Cray T3E or Compaq/Dec Sierra
#
#       make cleanup   - will remove all *.o files
#
###########################################################################

CC       = oshcc

# for Cray T3E, X1
# ARCH     = -D_CRAY
# OPTLEVEL = -O3 -DCRAY -I.
# for Compaq AlphaCluster
#ARCH     = -D__alpha
#OPTLEVEL = -O5 -arch host -tune host -fast
# SGI Altix
ARCH = -D_SGI -D_ALTIX
OPTLEVEL = -ggdb

# remove # on next line to print extra debug messages in M6, P6
# DEBUG    = -DDebug
# remove # on next line to get detailed pattern-match, solving time in M6, P6
#TIMER    = -DTimer
# remove # on next line to print bits for short test case in S6, P6
#BITS     = -DPrintBits
# remove # on next line to use popcnt subroutine in util.c
PCLZ     = -DSW_POPCNT

CFLAGS   = $(ARCH) $(OPTLEVEL) $(DEBUG) $(TIMER) $(BITS) $(PCLZ)

# remove # on next line to print details of last loop in R6
PRINTALL = -DPRINTALL

TARGET   = bench6

# for Cray X1
# LDFLAGS  = -O3 -DCRAY -I.
# for Alpha Sierra cluster
#LDFLAGS = 
# for Alpha Sierra cluster
#LDLIBS   = -lc -lm -lelan -lshmem
# for SGI Altix
LDFLAGS  = $(OPTLEVEL)
# LDLIBS   = -lsma -lmpi

SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)

all :  $(TARGET)

r6.o : r6.c
	$(CC) $(CFLAGS) $(PRINTALL) -c r6.c

$(TARGET) : $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(OBJECTS)

cleanall:	clean
	rm -f $(TARGET)
