! (c) 2011 University of Houston System.  All rights reserved. */
! 
! Tests shmem_broadcast32 shmem_broadcast64 calls
! PE 0 broadcasts to all other PEs
! source and destination arrays are shmalloc-ed
! 
program test_shmem_broadcast
  implicit none
  include 'mpp/shmem.fh'
  
  integer*8, save    :: pSync(SHMEM_BCAST_SYNC_SIZE)
   
  integer            :: i, success32, success64
  integer  , save    :: targ(2)
  integer  , save    :: src(2)
  integer*8, save    :: target(2)
  integer*8, save    :: source(2)
  integer            :: length, abort, errcode

  integer            :: me, npes

! Function definitions
  integer                   :: my_pe, num_pes

  call start_pes(0)
  me = my_pe()
  npes = num_pes()
  
  success32 = 0
  success64 = 0

  if(npes .eq. 2) then
    do i = 1, SHMEM_BCAST_SYNC_SIZE, 1
      pSync(i) = SHMEM_SYNC_VALUE
    end do 

    ! Test shmem_broadcast32
    do i = 1, npes, 1      
      src(i) = i + 1
    end do 

    do i = 1, npes, 1
      targ(i) = -999
    end do

    call shmem_barrier_all()

    call shmem_broadcast32(targ, src, npes, 0, 0, 0, npes, pSync)

    call shmem_barrier_all()

    if(me .eq. 1) then
      do i = 1, npes, 1
        if( targ(i) .ne. i+1) then
          success32 = 1
        end if
      end do

      if(success32 .eq. 1) then
        write (*,*) "Test shmem_broadcast32: Failed"
      else
        write (*,*) "Test shmem_broadcast32: Passed"
      end if
    end if

    call shmem_barrier_all()

    ! Test shmem_broadcast64

    do i = 1,npes, 1
      source(i) = i + 1
    end do

    do i = 0, npes, 1
      target(i) = -999
    end do

    call shmem_barrier_all()

    call shmem_broadcast64(target, source, npes, 0, 0, 0, npes, pSync)

    call shmem_barrier_all()

    if(me .eq. 1) then
      do i = 1, npes, 1
        if( target(i) .ne. (i+1)) then
          success64 = 1
        end if
      end do

      if(success64 .eq. 1) then
        write (*,*) "Test shmem_broadcast64: Failed"
      else
        write (*,*) "Test shmem_broadcast64: Passed"
      end if
    end if

  else
    write (*,*) "Number of PEs must be > 1 to test broadcast, test skipped"
  end if 

end program test_shmem_broadcast
