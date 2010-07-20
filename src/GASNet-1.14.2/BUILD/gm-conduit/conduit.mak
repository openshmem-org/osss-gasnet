#INSTRUCTIONS# Conduit-specific Makefile fragment settings
#INSTRUCTIONS#
#INSTRUCTIONS# The contents of this file are embedded into the 
#INSTRUCTIONS# *-(seq,par,parsync).mak Makefile fragments at conduit build time
#INSTRUCTIONS# The settings in those fragments are used to build GASNet clients
#INSTRUCTIONS# (including the GASNet tests). 
#INSTRUCTIONS# See the conduit-writer instructions in the generated fragments
#INSTRUCTIONS# or $(top_srcdir)/other/fragment-head.mak.in for usage info.

# When gm-conduit enables MPI compatibility, we must link with MPI compiler
#GASNET_LD_OVERRIDE = /opt/openmpi/gnu/1.4.2/bin/mpicc 
#GASNET_LDFLAGS_OVERRIDE =  -O3 --param max-inline-insns-single=35000 --param inline-unit-growth=10000 --param large-function-growth=200000 -Winline 
#MPI_COMPAT_INCLUDES = -I/home/tonyc/src/SHMEM/trunk/src/GASNet-1.14.2/other/mpi-spawner ###NOINSTALL### 
#MPI_COMPAT_LIBS = 

CONDUIT_LIBDIRS =  -L/usr/local/gm/lib
CONDUIT_LIBS = -lgm $(MPI_COMPAT_LIBS)
firehose_includes = -I/home/tonyc/src/SHMEM/trunk/src/GASNet-1.14.2/other/firehose ###NOINSTALL###
CONDUIT_INCLUDES = -I/usr/local/gm/include $(MPI_COMPAT_INCLUDES) $(firehose_includes)

