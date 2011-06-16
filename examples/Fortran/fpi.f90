!**********************************************************************
!   pi.f - compute pi by integrating f(x) = 4/(1 + x**2)     
!
!  (C) 2001 by Argonne National Laboratory.
!      See COPYRIGHT in top-level directory.
!     
!   Each node: 
!    1) receives the number of rectangles used in the approximation.
!    2) calculates the areas of it's rectangles.
!    3) Synchronizes for a global summation.
!   Node 0 prints the result.
!
!  Variables:
!
!    pi  the calculated result
!    n   number of points of integration.  
!    x           midpoint of each rectangle's interval
!    f           function to integrate
!    sum,pi      area of rectangles
!    tmp         temporary scratch space for global summation
!    i           do loop index
!****************************************************************************

function f(x)
  double precision x
  f = 4.d0 / (1.d0 + x*x)
end function

program main

  include 'mpp/shmem.fh'

  double precision  PI25DT
  parameter        (PI25DT = 3.141592653589793238462643d0)

  double precision, save :: mypi, pi
  double precision  h, sum, x, f, a
  integer, save :: n
  integer myid, numprocs, i, rc

  integer, dimension(SHMEM_BCAST_SYNC_SIZE), save :: psync
  real, dimension(SHMEM_REDUCE_MIN_WRKDATA_SIZE), save :: pwrk

  call start_pes(0)
  myid = my_pe()
  numprocs = num_pes()
! print *, "Process ", myid, " of ", numprocs, " is alive"

  psync = SHMEM_SYNC_VALUE
  call shmem_barrier_all


10 if ( myid .eq. 0 ) then
     write(6,98)
98   format('Enter the number of intervals: (0 quits)')
     read(5,99) n
99   format(i10)
  endif

  call shmem_broadcast4(n, n, 1, 0, 0, 0, numprocs, psync)

  !                                 check for quit signal
  if ( n .le. 0 ) goto 30

  !                                 calculate the interval size
  h = 1.0d0/n

  sum  = 0.0d0
  do i = myid+1, n, numprocs
     x = h * (dble(i) - 0.5d0)
     sum = sum + f(x)
  end do
  mypi = h * sum

  !                                 collect all the partial sums
  call shmem_barrier_all
  call shmem_real8_sum_to_all(pi, mypi, 1, 0, 0, numprocs, pwrk, psync)

  !                                 node 0 prints the answer.
  if (myid .eq. 0) then
     write(6, 97) pi, abs(pi - PI25DT)
97   format('  pi is approximately: ', F18.16, '  Error is: ', F18.16)
  endif

  goto 10

30 stop

end program main
