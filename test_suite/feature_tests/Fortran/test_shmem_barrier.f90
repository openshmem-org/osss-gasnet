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

! Tests shmem_barrier() and shmem_barrier_all

program test_shmem_barrier
  implicit none

  include 'mpp/shmem.fh'

  integer, save :: pSync(SHMEM_BCAST_SYNC_SIZE)
  integer, save :: x

  integer       :: me, npes, i

  ! Function definitions
  integer                   :: my_pe, num_pes
  !

  x = 10101
  
  call start_pes(0);

  me   = my_pe();
  npes = num_pes();

  do i = 1, SHMEM_BCAST_SYNC_SIZE
    pSync(i) = SHMEM_SYNC_VALUE
  end do

  ! Make sure this job is running on at least 2 PEs.

  if (npes .gt. 1) then

    call shmem_integer_put(x, 4, 1, mod((me + 1), npes))

    call shmem_barrier_all()

    if(me .eq. npes - 1) then
      if(x .eq. 4) then
        write (*,*) 'Test shmem_barrier_all: Passed'
      else
        write (*,*) 'Test shmem_barrier_all: Failed'
      end if
    end if

    x = -9
    call shmem_barrier_all()

    if(me .eq. 0 .OR. me .eq. 1) then

      if(me .eq. 0) then
        call shmem_integer_put(x, 4, 1, 1)
      end if
  
      call shmem_barrier(0, 0, 2, pSync)
      
      if(me .eq. 1) then
        if(x .eq. 4) then
          write (*,*) 'Test shmem_barrier: Passed'
        else
          write (*,*) 'Test shmem_barrier: Failed'
        end if
      end if
      
    end if
  else
    write (*,*) 'Number of PEs must be > 1 to test barrier, test skipped'

  end if  
end program test_shmem_barrier
