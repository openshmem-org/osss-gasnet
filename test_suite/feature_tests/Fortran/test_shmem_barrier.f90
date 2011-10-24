! (c) 2011 University of Houston System.  All rights reserved. 
! Tests shmem_barrier call

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
