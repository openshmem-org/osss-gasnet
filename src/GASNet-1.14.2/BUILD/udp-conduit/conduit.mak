#INSTRUCTIONS# Conduit-specific Makefile fragment settings
#INSTRUCTIONS#
#INSTRUCTIONS# The contents of this file are embedded into the 
#INSTRUCTIONS# *-(seq,par,parsync).mak Makefile fragments at conduit build time
#INSTRUCTIONS# The settings in those fragments are used to build GASNet clients
#INSTRUCTIONS# (including the GASNet tests). 
#INSTRUCTIONS# See the conduit-writer instructions in the generated fragments
#INSTRUCTIONS# or $(top_srcdir)/other/fragment-head.mak.in for usage info.

# AMUDP is C++-based, which requires us to link using the C++ compiler
GASNET_LD_OVERRIDE = /opt/local/gcc/4.5.0/bin/g++ 
GASNET_LDFLAGS_OVERRIDE = -O2  

# hooks for using AMUDP library from within build tree ###NOINSTALL###
# (nothing additional required for installed copy)     ###NOINSTALL###
CONDUIT_INCLUDES = -I/home/tonyc/src/SHMEM/trunk/src/GASNet-1.14.2/other/amudp          ###NOINSTALL###
CONDUIT_LIBDIRS =  -L/home/tonyc/src/SHMEM/trunk/src/GASNet-1.14.2/BUILD/other/amudp        ###NOINSTALL###

CONDUIT_LIBS = -lamudp   
