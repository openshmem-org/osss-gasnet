# $Header: /var/local/cvs/gasnet/other/fragment-body.mak.in,v 1.7 2009/09/18 23:33:36 phargrov Exp $
# ----------------------------------------------------------------------
# Following section other/fragment-body.mak.  Generated from fragment-body.mak.in by configure.

# ----------------------------------------------------------------------
# Directory-based options

GASNET_INCLUDES =  -I###INSTALL_PREFIX###/include -I###INSTALL_PREFIX###/include/#conduit_name#-conduit $(CONDUIT_INCLUDES) $(CONDUIT_INCLUDES_#THREAD_MODEL#)
GASNET_LIBDIRS = -L###INSTALL_PREFIX###/lib $(CONDUIT_LIBDIRS) $(CONDUIT_LIBDIRS_#THREAD_MODEL#)

# Textual lines containing the string "###NOINSTALL###" are removed by the install process
# (must be one continuous line) ###NOINSTALL###
GASNET_INCLUDES =  -I/home/tonyc/src/SHMEM/trunk/src/GASNet-1.14.2 -I/home/tonyc/src/SHMEM/trunk/src/GASNet-1.14.2/#conduit_name#-conduit -I/home/tonyc/src/SHMEM/trunk/src/GASNet-1.14.2/other $(CONDUIT_INCLUDES) $(CONDUIT_INCLUDES_#THREAD_MODEL#) -I/home/tonyc/src/SHMEM/trunk/src/GASNet-1.14.2/extended-ref -I/home/tonyc/src/SHMEM/trunk/src/GASNet-1.14.2/BUILD  ###NOINSTALL###
GASNET_LIBDIRS = -L/home/tonyc/src/SHMEM/trunk/src/GASNet-1.14.2/BUILD/#conduit_name#-conduit $(CONDUIT_LIBDIRS) $(CONDUIT_LIBDIRS_#THREAD_MODEL#) ###NOINSTALL###

# ----------------------------------------------------------------------
# C compiler and options

GASNET_CC = /usr/lib64/ccache/gcc 

GASNET_OPT_CFLAGS = -O3 --param max-inline-insns-single=35000 --param inline-unit-growth=10000 --param large-function-growth=200000 -Winline $(CONDUIT_OPT_CFLAGS) $(CONDUIT_OPT_CFLAGS_#THREAD_MODEL#)
GASNET_MISC_CFLAGS =  $(CONDUIT_MISC_CFLAGS) $(CONDUIT_MISC_CFLAGS_#THREAD_MODEL#)
GASNET_MISC_CPPFLAGS =  $(CONDUIT_MISC_CPPFLAGS) $(CONDUIT_MISC_CPPFLAGS_#THREAD_MODEL#)

GASNET_EXTRADEFINES_SEQ = 
GASNET_EXTRADEFINES_PAR = -D_REENTRANT -D_GNU_SOURCE
GASNET_EXTRADEFINES_PARSYNC = -D_REENTRANT -D_GNU_SOURCE

GASNET_DEFINES = -DGASNET_#THREAD_MODEL# $(GASNET_EXTRADEFINES_#THREAD_MODEL#) $(CONDUIT_DEFINES) $(CONDUIT_DEFINES_#THREAD_MODEL#)

# ----------------------------------------------------------------------
# Documented compilation convenience aliases

GASNET_CFLAGS = $(GASNET_OPT_CFLAGS) $(GASNET_MISC_CFLAGS) $(MANUAL_CFLAGS)
GASNET_CPPFLAGS = $(GASNET_MISC_CPPFLAGS) $(GASNET_DEFINES) $(GASNET_INCLUDES)

# ----------------------------------------------------------------------
# linker and options

GASNET_LD = $(GASNET_LD_OVERRIDE)

# linker flags that GASNet clients should use 
GASNET_LDFLAGS = $(GASNET_LDFLAGS_OVERRIDE) $(CONDUIT_LDFLAGS) $(CONDUIT_LDFLAGS_#THREAD_MODEL#) $(MANUAL_LDFLAGS)

GASNET_EXTRALIBS_SEQ = 
GASNET_EXTRALIBS_PAR = -lpthread
GASNET_EXTRALIBS_PARSYNC = -lpthread

# libraries that GASNet clients should append to link line
GASNET_LIBS =                             \
    $(GASNET_LIBDIRS)                     \
    -lgasnet-#conduit_name#-#thread_model# \
    $(CONDUIT_LIBS)                       \
    $(CONDUIT_LIBS_#THREAD_MODEL#)        \
    $(GASNET_EXTRALIBS_#THREAD_MODEL#)    \
                        \
    -L/usr/lib/gcc/x86_64-redhat-linux/4.1.2 -lgcc                              \
                                    \
    -lm                                \
    $(MANUAL_LIBS)

# ----------------------------------------------------------------------
