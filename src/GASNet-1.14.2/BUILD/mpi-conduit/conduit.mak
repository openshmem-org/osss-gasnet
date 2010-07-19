#INSTRUCTIONS# Conduit-specific Makefile fragment settings
#INSTRUCTIONS#
#INSTRUCTIONS# The contents of this file are embedded into the 
#INSTRUCTIONS# *-(seq,par,parsync).mak Makefile fragments at conduit build time
#INSTRUCTIONS# The settings in those fragments are used to build GASNet clients
#INSTRUCTIONS# (including the GASNet tests). 
#INSTRUCTIONS# See the conduit-writer instructions in the generated fragments
#INSTRUCTIONS# or $(top_srcdir)/other/fragment-head.mak.in for usage info.

# AMMPI is MPI-based, which requires us to link using the system MPI compiler
GASNET_LD_OVERRIDE = /opt/openmpi/gnu/1.4.2/bin/mpicc 
GASNET_LDFLAGS_OVERRIDE =  -O3 --param max-inline-insns-single=35000 --param inline-unit-growth=10000 --param large-function-growth=200000 -Winline 

# hooks for using AMMPI library from within build tree ###NOINSTALL### 
# (nothing additional required for installed copy)     ###NOINSTALL###
CONDUIT_INCLUDES = -I/home/tonyc/src/GASNet-1.14.2/other/ammpi          ###NOINSTALL###
CONDUIT_LIBDIRS =  -L/home/tonyc/src/GASNet-1.14.2/BUILD/other/ammpi        ###NOINSTALL###

CONDUIT_LIBS = -lammpi 
