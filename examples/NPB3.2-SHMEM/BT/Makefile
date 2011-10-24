SHELL=/bin/sh
BENCHMARK=bt
BENCHMARKU=BT
#SUBTYPE=FULL

include ../config/make.def


OBJS = bt.o make_set.o initialize.o exact_solution.o exact_rhs.o \
       set_constants.o adi.o define.o copy_faces.o rhs.o      \
       x_solve.o y_solve.o z_solve.o add.o error.o verify.o setup_mpi.o \
       ${COMMON}/print_results.o ${COMMON}/timers.o ${COMMON}/${RAND}.o

include ../sys/make.common

# npbparams.h is included by header.h
# The following rule should do the trick but many make programs (not gmake)
# will do the wrong thing and rebuild the world every time (because the
# mod time on header.h is not changed. One solution would be to 
# touch header.h but this might cause confusion if someone has
# accidentally deleted it. Instead, make the dependency on npbparams.h
# explicit in all the lines below (even though dependence is indirect). 

# header.h: npbparams.h

${PROGRAM}: config ${OBJS}
	@if [ x$(SUBTYPE) = xfull ] ; then		\
		${MAKE} PROGRAM=${PROGRAM} bt-full;	\
	elif [ x$(SUBTYPE) = xFULL ] ; then		\
		${MAKE} PROGRAM=${PROGRAM} bt-full;	\
	elif [ x$(SUBTYPE) = xsimple ] ; then		\
		${MAKE} PROGRAM=${PROGRAM} bt-simple;	\
	elif [ x$(SUBTYPE) = xSIMPLE ] ; then		\
		${MAKE} PROGRAM=${PROGRAM} bt-simple;	\
	elif [ x$(SUBTYPE) = xfortran ] ; then		\
		${MAKE} PROGRAM=${PROGRAM} bt-fortran;	\
	elif [ x$(SUBTYPE) = xFORTRAN ] ; then		\
		${MAKE} PROGRAM=${PROGRAM} bt-fortran;	\
	elif [ x$(SUBTYPE) = xepio ] ; then		\
		${MAKE} PROGRAM=${PROGRAM} bt-epio;	\
	elif [ x$(SUBTYPE) = xEPIO ] ; then		\
		${MAKE} PROGRAM=${PROGRAM} bt-epio;	\
	else						\
		${MAKE} PROGRAM=${PROGRAM} bt-bt;		\
	fi

bt-bt: ${OBJS} btio.o
	${FLINK} ${FLINKFLAGS} -o ${PROGRAM} ${OBJS} btio.o ${FMPI_LIB}

bt-full: ${OBJS} full_mpiio.o btio_common.o
	${FLINK} ${FLINKFLAGS} -o ${PROGRAM}.mpi_io_full ${OBJS} btio_common.o full_mpiio.o ${FMPI_LIB}

bt-simple: ${OBJS} simple_mpiio.o btio_common.o
	${FLINK} ${FLINKFLAGS} -o ${PROGRAM}.mpi_io_simple ${OBJS} btio_common.o simple_mpiio.o ${FMPI_LIB}

bt-fortran: ${OBJS} fortran_io.o btio_common.o
	${FLINK} ${FLINKFLAGS} -o ${PROGRAM}.fortran_io ${OBJS} btio_common.o fortran_io.o ${FMPI_LIB}

bt-epio: ${OBJS} epio.o btio_common.o
	${FLINK} ${FLINKFLAGS} -o ${PROGRAM}.ep_io ${OBJS} btio_common.o epio.o ${FMPI_LIB}

.f.o:
	${FCOMPILE} $<

.c.o:
	${CCOMPILE} $<


bt.o:             bt.f  header.h npbparams.h  mpinpb.h
make_set.o:       make_set.f  header.h npbparams.h  mpinpb.h
initialize.o:     initialize.f  header.h npbparams.h
exact_solution.o: exact_solution.f  header.h npbparams.h
exact_rhs.o:      exact_rhs.f  header.h npbparams.h
set_constants.o:  set_constants.f  header.h npbparams.h
io.o:             io.f  header.h npbparams.h
adi.o:            adi.f  header.h npbparams.h
define.o:         define.f  header.h npbparams.h
copy_faces.o:     copy_faces.f  header.h npbparams.h  mpinpb.h
rhs.o:            rhs.f  header.h npbparams.h
x_solve.o:        x_solve.f  header.h npbparams.h  mpinpb.h
ninvr.o:          ninvr.f  header.h npbparams.h
y_solve.o:        y_solve.f  header.h npbparams.h  mpinpb.h
pinvr.o:          pinvr.f  header.h npbparams.h
z_solve.o:        z_solve.f  header.h npbparams.h  mpinpb.h
tzetar.o:         tzetar.f  header.h npbparams.h
add.o:            add.f  header.h npbparams.h
txinvr.o:         txinvr.f  header.h npbparams.h
error.o:          error.f  header.h npbparams.h  mpinpb.h
verify.o:         verify.f  header.h npbparams.h  mpinpb.h
setup_mpi.o:      setup_mpi.f mpinpb.h npbparams.h 
btio_common.o:    btio_common.f mpinpb.h npbparams.h 
fortran_io.o:     fortran_io.f mpinpb.h npbparams.h 
simple_mpiio.o:   simple_mpiio.f mpinpb.h npbparams.h 
full_mpiio.o:     full_mpiio.f mpinpb.h npbparams.h 
epio.o:           epio.f mpinpb.h npbparams.h 

clean:
	- rm -f *.o *~ mputil*
	- rm -f  npbparams.h core
