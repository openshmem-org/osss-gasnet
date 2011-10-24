c NPROCS = 4 CLASS = A
c  
c  
c  This file is generated automatically by the setparams utility.
c  It sets the number of processors and the class of the NPB
c  in this directory. Do not modify it by hand.
c  
        integer            na, nonzer, niter
        double precision   shift, rcond
        parameter(  na=14000,
     >              nonzer=11,
     >              niter=15,
     >              shift=20.,
     >              rcond=1.0d-1 )

c number of nodes for which this version is compiled
        integer    nnodes_compiled
        parameter( nnodes_compiled = 4)
        integer    num_proc_cols, num_proc_rows
        parameter( num_proc_cols=2, num_proc_rows=2 )
        logical  convertdouble
        parameter (convertdouble = .false.)
        character*11 compiletime
        parameter (compiletime='24 Oct 2011')
        character*3 npbversion
        parameter (npbversion='3.2')
        character*7 cs1
        parameter (cs1='oshfort')
        character*9 cs2
        parameter (cs2='$(MPIF77)')
        character*6 cs3
        parameter (cs3='(none)')
        character*6 cs4
        parameter (cs4='(none)')
        character*12 cs5
        parameter (cs5='-O3 -g -I./ ')
        character*6 cs6
        parameter (cs6='-O3 -g')
        character*6 cs7
        parameter (cs7='randi8')
