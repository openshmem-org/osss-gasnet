#INSTRUCTIONS# Conduit-specific Makefile fragment settings
#INSTRUCTIONS#
#INSTRUCTIONS# The contents of this file are embedded into the 
#INSTRUCTIONS# *-(seq,par,parsync).mak Makefile fragments at conduit build time
#INSTRUCTIONS# The settings in those fragments are used to build GASNet clients
#INSTRUCTIONS# (including the GASNet tests). 
#INSTRUCTIONS# See the conduit-writer instructions in the generated fragments
#INSTRUCTIONS# or $(top_srcdir)/other/fragment-head.mak.in for usage info.

CONDUIT_LIBDIRS = -L/bgsys/drivers/ppcfloor/comm/sys/lib -L/bgsys/drivers/ppcfloor/runtime/SPI
CONDUIT_LIBS = -ldcmf-fast.cnk -ldcmfcoll-fast.cnk -lpthread -lrt -lSPI-fast.cna 
