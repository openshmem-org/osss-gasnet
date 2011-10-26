!
! Copyright (c) 2011, University of Houston System and Oak Ridge National
! Loboratory.
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
!   National Loboratory nor the names of its contributors may be used to
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

! Attempts to allocate an array in the symmetric heap.
! The test passes if we can correctly PUT data into that object on a remote PE.
program test_shmem_shpalloc
  implicit none
  include 'mpp/shmem.fh'

  integer, parameter :: N = 3

  integer*8          :: ptr
  integer            :: dst(1)    
  pointer            (ptr, dst)
  integer            :: src(N)
  integer            :: success ! this should probably be renamed to failed :)

  integer            :: errcode, abort, me, npes, i

! Function definitons
  integer    :: my_pe, num_pes

  call start_pes(0)

  me  = my_pe()
  npes = num_pes()

  call shpalloc(ptr, N, errcode, abort)

  do i = 1, N, 1
    dst(i) = -9
    src(i) = me
  end do

  call shmem_barrier_all()

  if(me .eq. 1) then
    call shmem_integer_put(dst, src, N, 0)
  end if

  call shmem_barrier_all()

  success = 0
  if(me .eq. 0) then  	
    do i = 1, N, 1
      if(dst(i) .ne. 1) then
        success = 1
      end if
    end do
  end if

	if(me .eq. 0) then
	  if(success .eq. 0) then
	    write (*,*) "Test shpalloc(): Passed"
	  else
	    write (*,*) "Test shpalloc(): Failed"
	  end if
	end if
	
  call shmem_barrier_all()

  call shpdeallc(ptr, errcode, abort)
  
  ! TODO: test if shpdeallc really deallocates memory

end program
