PREFIX          = $(HOME)/SHMEM

# ---------------------------------------------------------

GASNET          = $(HOME)/gasnet/everything

include $(GASNET)/include/mpi-conduit/mpi-par.mak 

# ---------------------------------------------------------

CC              = gcc

DEBUG_AND_OPT   = -g -O3

COMMON_FLAGS    = $(CPPFLAGS) $(CFLAGS) $(DEBUG_AND_OPT) 

am:	am.o
	$(GASNET_LD) -o $@ $^ $(GASNET_LIBS)

am.o:	am.c
	$(GASNET_CC) \
		$(GASNET_CPPFLAGS) $(GASNET_CFLAGS) \
		$(COMMON_FLAGS) -c -o $@ $<
