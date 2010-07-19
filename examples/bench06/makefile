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

# remove # on next line to print extra debug messages in M6, P6
#DEBUG    = -DDebug
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
# LDLIBS   = -lc -lm -lsma

SUBOBJS = s6.o p6.o p6solve.o c6.o r6.o prandom.o util.o

all :  ${TARGET}

m6.o : m6.c
	${CC} ${CFLAGS} -c m6.c
s6.o : s6.c
	${CC} ${CFLAGS} -c s6.c
p6.o : p6.c
	${CC} ${CFLAGS} -c p6.c
p6solve.o : p6solve.c
	${CC} ${CFLAGS} -c p6solve.c
c6.o : c6.c
	${CC} ${CFLAGS} -c c6.c
r6.o : r6.c
	${CC} ${CFLAGS} $(PRINTALL) -c r6.c
prandom.o : prandom.c
	${CC} $(CFLAGS) -c prandom.c
util.o : util.c
	${CC} $(CFLAGS) -c util.c

bench6 : m6.o ${SUBOBJS}
	${CC} ${LDFLAGS} -o bench6 m6.o ${SUBOBJS} ${LDLIBS}

clean:
	rm -f *6.o p6solve.o

cleanall:
	rm -f *.o

