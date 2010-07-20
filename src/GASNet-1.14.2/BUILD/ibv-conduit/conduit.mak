#INSTRUCTIONS# Conduit-specific Makefile fragment settings
#INSTRUCTIONS#
#INSTRUCTIONS# The contents of this file are embedded into the 
#INSTRUCTIONS# *-(seq,par,parsync).mak Makefile fragments at conduit build time
#INSTRUCTIONS# The settings in those fragments are used to build GASNet clients
#INSTRUCTIONS# (including the GASNet tests). 
#INSTRUCTIONS# See the conduit-writer instructions in the generated fragments
#INSTRUCTIONS# or $(top_srcdir)/other/fragment-head.mak.in for usage info.

# When ibv-conduit uses an MPI-based bootstrapper, we must
# link using the system MPI compiler
GASNET_LD_OVERRIDE = /opt/openmpi/gnu/1.4.2/bin/mpicc 
GASNET_LDFLAGS_OVERRIDE =  -O3 --param max-inline-insns-single=35000 --param inline-unit-growth=10000 --param large-function-growth=200000 -Winline 
MPI_COMPAT_LIBS = 

CONDUIT_INCLUDES = -DGASNET_CONDUIT_IBV
CONDUIT_INCLUDES = -DGASNET_CONDUIT_IBV -I/home/tonyc/src/SHMEM/trunk/src/GASNet-1.14.2/other/firehose -I/home/tonyc/src/SHMEM/trunk/src/GASNet-1.14.2/vapi-conduit ###NOINSTALL###

CONDUIT_LIBDIRS = -L/usr/lib64
CONDUIT_LIBS = -libverbs $(MPI_COMPAT_LIBS)

# If ibv-conduit has internal conduit threads, then it needs 
# threading flags and libs - even in GASNET_SEQ mode
#CONDUIT_DEFINES_SEQ = -D_REENTRANT -D_GNU_SOURCE
#CONDUIT_LIBS_SEQ =    -lpthread

