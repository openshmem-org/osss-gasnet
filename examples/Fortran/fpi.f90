!
!
! Copyright (c) 2011 - 2014
!   University of Houston System and Oak Ridge National Laboratory.
! 
! All rights reserved.
! 
! Redistribution and use in source and binary forms, with or without
! modification, are permitted provided that the following conditions
! are met:
! 
! o Redistributions of source code must retain the above copyright notice,
!   this list of conditions and the following disclaimer.
! 
! o Redistributions in binary form must reproduce the above copyright
!   notice, this list of conditions and the following disclaimer in the
!   documentation and/or other materials provided with the distribution.
! 
! o Neither the name of the University of Houston System, Oak Ridge
!   National Laboratory nor the names of its contributors may be used to
!   endorse or promote products derived from this software without specific
!   prior written permission.
! 
! THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
! "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
! LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
! A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
! HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
! SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
! TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
! PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
! LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
! NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
! SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
!


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

  integer :: num_pes, my_pe

  double precision  PI25DT
  parameter        (PI25DT = 3.141592653589793238462643d0)

  double precision, save :: mypi, pi
  double precision       :: h, sum, x, a
  integer, save          :: n
  integer                :: myid, numprocs, i, rc

  integer :: psync_bcast(SHMEM_BCAST_SYNC_SIZE)
  data psync_bcast /SHMEM_BCAST_SYNC_SIZE*SHMEM_SYNC_VALUE/

  integer :: psync_reduce(SHMEM_REDUCE_SYNC_SIZE)
  data psync_reduce /SHMEM_REDUCE_SYNC_SIZE*SHMEM_SYNC_VALUE/

  real(kind=8), dimension(SHMEM_REDUCE_MIN_WRKDATA_SIZE), save :: pwrk

  call start_pes(0)
  myid = my_pe()
  numprocs = num_pes()
! print *, "Process ", myid, " of ", numprocs, " is alive"

  call shmem_barrier_all

10 if ( myid .eq. 0 ) then
     write(6,98)
98   format('Enter the number of intervals: (0 quits)')
     read(5,99) n
99   format(i10)
  endif

  call shmem_broadcast4(n, n, 1, 0, 0, 0, numprocs, psync_bcast)

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
  call shmem_real8_sum_to_all(pi, mypi, 1, 0, 0, numprocs, pwrk, psync_reduce)

  !                                 node 0 prints the answer.
  if (myid .eq. 0) then
     write(6, 97) pi, abs(pi - PI25DT)
97   format('  pi is approximately: ', F18.16, '  Error is: ', F18.16)
  endif

  goto 10

30 stop

end program main
