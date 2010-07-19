# This Makefile fragment is used to build GASNet libraries
# it is not meant to be used directly 
# other/Makefile-libgasnet.mak.  Generated from Makefile-libgasnet.mak.in by configure.

.PHONY: do-libgasnet-seq do-libgasnet-par do-libgasnet-parsync  \
        do-libgasnet_tools-seq do-libgasnet_tools-par do-libgasnet_tools  \
        do-libgasnet check-exports do-pthreads-error 

VPATH=$(srcdir)

thread_defines = -D_REENTRANT -D_GNU_SOURCE
SEPARATE_CC = 

PLPA_INCLUDES = -I$(top_srcdir)/other/plpa/src/libplpa
PLPA_SOURCES  = $(top_srcdir)/other/plpa/src/libplpa/plpa_api_probe.c \
                $(top_srcdir)/other/plpa/src/libplpa/plpa_dispatch.c

#PSHM_SOURCES  = $(top_srcdir)/gasnet_pshm.c

TOOLLIBINCLUDES =         \
          \
        -I$(srcdir)       \
        -I$(top_srcdir)   \
        -I$(top_builddir) \
        -I$(top_srcdir)/other \
	$(PLPA_INCLUDES)

LIBINCLUDES = $(TOOLLIBINCLUDES) \
        -I$(top_srcdir)/extended-ref  


TOOLLIBDEFINES =                    \
        $(LIBGASNET_THREAD_DEFINES) \
         

LIBDEFINES =                        \
	$(TOOLLIBDEFINES)	    \
        -DGASNET_$(THREAD_MODEL)    

 TOOLLIB_DEBUGFLAGS = -DNDEBUG
TOOLLIBCFLAGS =                   \
        -DGASNETT_BUILDING_TOOLS  \
        -O3 --param max-inline-insns-single=35000 --param inline-unit-growth=10000 --param large-function-growth=200000 -Winline                  \
                     \
        $(TOOLLIBDEFINES)         \
	$(TOOLLIB_DEBUGFLAGS)     \
	$(TOOLLIBINCLUDES)	  \
	$${keeptmps:+-save-temps} \
	$(MANUAL_LIBCFLAGS)

LIBCFLAGS =                       \
        -O3 --param max-inline-insns-single=35000 --param inline-unit-growth=10000 --param large-function-growth=200000 -Winline                  \
                     \
        $(LIBDEFINES)             \
        $(CONDUIT_EXTRALIBCFLAGS) \
	$(LIBINCLUDES)		  \
	$${keeptmps:+-save-temps} \
	$(MANUAL_LIBCFLAGS)

libgasnet_tools_sources =            \
	$(top_srcdir)/gasnet_tools.c \
	$(PLPA_SOURCES)

libgasnet_sources =                                         \
        $(CONDUIT_SOURCELIST)                               \
        $(libgasnet_tools_sources)                          \
        $(PSHM_SOURCES)                                     \
        $(top_srcdir)/extended-ref/gasnet_extended_refvis.c \
        $(top_srcdir)/extended-ref/gasnet_extended_refcoll.c\
        $(top_srcdir)/extended-ref/gasnet_coll_putget.c     \
        $(top_srcdir)/extended-ref/gasnet_coll_eager.c      \
        $(top_srcdir)/extended-ref/gasnet_coll_rvous.c      \
        $(top_srcdir)/extended-ref/gasnet_coll_team.c				\
        $(top_srcdir)/extended-ref/gasnet_coll_hashtable.c   \
        $(top_srcdir)/gasnet_internal.c                     \
        $(top_srcdir)/gasnet_trace.c                        \
        $(top_srcdir)/gasnet_mmap.c                         \
	$(top_srcdir)/gasnet_diagnostic.c

libgasnet_objects = \
	`for file in $(libgasnet_sources) ; do echo \`basename $$file .c\`.o ; done` \
	$(CONDUIT_SPECIAL_OBJS)

libgasnet_tools_dependencies =  \
        $(CONFIG_HEADER)        \
        $(top_srcdir)/*.[ch]    \
        $(top_srcdir)/other/*.h

libgasnet_dependencies =                  \
        $(libgasnet_tools_dependencies)   \
        $(srcdir)/*.[ch]                  \
        $(top_srcdir)/extended-ref/*.[ch] \
        $(top_srcdir)/other/smp-collectives/*.[ch] \
	$(CONDUIT_SOURCELIST)             \
	$(CONDUIT_EXTRAHEADERS)           \
	$(CONDUIT_EXTRADEPS)

# library targets 
THREAD_MODEL=SEQ
THREAD_MODEL_LC=`echo "$(THREAD_MODEL)" | gawk '{print tolower($$0)}'`
LIBGASNET_NAME=libgasnet-$(CONDUIT_NAME)
do-libgasnet: $(CONDUIT_SPECIAL_OBJS)
	@mkdir -p .$(THREAD_MODEL)
	@libgasnet_objects="$(libgasnet_objects)" ; libgasnet_objects=`echo $$libgasnet_objects` ; \
	pwd=`/bin/pwd`; keeptmps='$(KEEPTMPS)'; \
	if test -z '$(KEEPTMPS)'; then rmcmd="&& rm -f $$libgasnet_objects"; fi; \
	if test -n '$(SEPARATE_CC)' ; then \
	  compcmd="for file in $(libgasnet_sources) ; do $(CC) $(LIBCFLAGS) -c "'$$file'" || exit "'$$?'" ; done" ; \
	else \
	  compcmd="$(CC) $(LIBCFLAGS) -c $(libgasnet_sources)" ; \
	fi ; \
	cmd="$$compcmd && \
	$(AR) cru $$pwd/$(LIBGASNET_NAME)-$(THREAD_MODEL_LC).a $$libgasnet_objects && \
	$(RANLIB) $$pwd/$(LIBGASNET_NAME)-$(THREAD_MODEL_LC).a $$rmcmd"; \
	echo " --- BUILDING $(LIBGASNET_NAME)-$(THREAD_MODEL_LC).a --- " ; \
        echo $$cmd ; cd .$(THREAD_MODEL) ; eval $$cmd
	@test -n '$(KEEPTMPS)' || rm -Rf .$(THREAD_MODEL)

set_dirs = top_srcdir=`cd $(top_srcdir); /bin/pwd`         \
           top_builddir=`cd $(top_builddir); /bin/pwd`     \
           srcdir=`cd $(srcdir); /bin/pwd`                 \
           builddir=`/bin/pwd`                             

do-libgasnet-seq: $(libgasnet_dependencies) $(CONDUIT_SEQ_HOOK)
	@$(MAKE) THREAD_MODEL=SEQ                            \
	  LIBGASNET_THREAD_DEFINES=                          \
          $(set_dirs) do-libgasnet

do-libgasnet-par: $(libgasnet_dependencies) $(CONDUIT_PAR_HOOK)
	@#$(MAKE) do-pthreads-error
	@$(MAKE) THREAD_MODEL=PAR                            \
          LIBGASNET_THREAD_DEFINES="$(thread_defines)"       \
          $(set_dirs) do-libgasnet

do-libgasnet-parsync: $(libgasnet_dependencies) $(CONDUIT_PARSYNC_HOOK)
	@#$(MAKE) do-pthreads-error
	@$(MAKE) THREAD_MODEL=PARSYNC                        \
          LIBGASNET_THREAD_DEFINES="$(thread_defines)"       \
          $(set_dirs) do-libgasnet

do-libgasnet_tools:
	@keeptmps="$(KEEPTMPS)" ;                            \
	 $(MAKE)                                             \
	  LIBCFLAGS="$(TOOLLIBCFLAGS)"                       \
          LIBGASNET_NAME=libgasnet_tools		     \
	  libgasnet_sources="$(libgasnet_tools_sources)"     \
          do-libgasnet

do-libgasnet_tools-seq: $(libgasnet_tools_dependencies)
	@$(MAKE) THREAD_MODEL=SEQ                            \
	  LIBGASNET_THREAD_DEFINES=                          \
          $(set_dirs) do-libgasnet_tools

do-libgasnet_tools-par: $(libgasnet_tools_dependencies)
	@#$(MAKE) do-pthreads-error
	@$(MAKE) THREAD_MODEL=PAR                            \
          LIBGASNET_THREAD_DEFINES="$(thread_defines) -DGASNETT_THREAD_SAFE" \
          $(set_dirs) do-libgasnet_tools


do-pthreads-error: 
	@echo "ERROR: pthreads support was not detected at configure time"
	@echo "       try re-running configure with --enable-pthreads"
	@exit 1

# bug1613: avoid automake infinite recursion here, because top-level Makefile includes this
# fragment and also provides the rules for rebuilding config.status
#cd $(top_builddir)/other && $(MAKE) Makefile-libgasnet.mak
$(top_builddir)/other/Makefile-libgasnet.mak: $(top_srcdir)/other/Makefile-libgasnet.mak.in
	cd $(top_builddir) && CONFIG_FILES=other/Makefile-libgasnet.mak CONFIG_HEADERS= ./config.status

check-exports: $(libraries)
	@echo Checking libgasnet exports...
	@if test x$(CHECK_EXPORTS) = x0; then                                       \
	  echo Skipped by user request ;                                            \
	  exit 0 ;                                                                  \
	 fi ;                                                                       \
	 failed=0 ;                                                                 \
	 for lib in $(libraries) ; do                                               \
	  echo ;                                                                    \
	  echo $$lib: ;                                                             \
	  /usr/bin/nm --defined-only $$lib |                                               \
	    grep -v -e ' [\._]*gasnetc_' -e ' [\._]*gasneti_' -e ' [\._]*gasnete_'  \
		    -e ' [\._]*gasnet_' -e ' [\._]*gasnett_'                        \
		    -e ' [\._]*fh_' -e ' [\._]*firehose_'                           \
		    -e ' [\._]*fhi_' -e ' [\._]*fhc_' -e ' [\._]*fhsmp_' -e ' [\._]*fhuni_' \
		    -e ' [\._]*myxml_' -e ' [\._]*smp_coll_'                        \
		    -e ' W _bgp_Make_[a-zA-Z]*_UCI' -e ' [\._]*emutls_'             \
		    -e __FUNCTION__ -e __PRETTY_FUNCTION__ -e ' [\._]*DWinfo'       \
               -e ' [\._]*debug_info_seg' -e ' [\._]*debug_abbrev_seg'         \
               -e ' [\._]*debug_frame_seg' -e ' [\._]*debug_line_seg'          \
               -e ' [\._]*stab' -e ' [\._]*gnu_dev_' -e '^00* W '  |           \
	    /usr/bin/perl -n -e 'print if /^[0-9a-fA-F]+\s+[A-Z]\s+/' > .$$lib.exp;        \
	  if test -s .$$lib.exp ; then                                              \
	    cat .$$lib.exp ;                                                        \
	    echo FAILED ;                                                           \
	    failed=1 ;                                                              \
	  else                                                                      \
	    echo PASSED ;                                                           \
	  fi ;                                                                      \
	  rm -f .$$lib.exp ;                                                        \
	done ; exit $$failed

#check-exports: $(libraries)
#	@echo check-exports test SKIPPED

