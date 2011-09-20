! (c) 2011 University of Houston System.  All rights reserved. */
! Tests shmem_fcollect4/8 shmem_collect4/8 
! Each PE contributes 4 elements 
! 
program test_shmem_collects
  implicit none
  include 'mpp/shmem.fh'
 
  integer  , parameter :: dst_size = 8 ! assuming 2 pes ( 2 x 4 elements)

  integer,   save :: pSync(SHMEM_BCAST_SYNC_SIZE)

  integer*4, save :: src1(4) = (/ 11, 12, 13, 14 /) 
  integer*8, save :: src2(4) = (/ 101, 102, 103, 104 /)
  integer*4, save :: src3(4) = (/ 1, 2, 3, 4 /)
  integer*8, save :: src4(4) = (/ 11, 12, 13, 14 /)

  integer         :: npes
  integer         :: me

  integer,   save :: x

  integer         :: i, j, k, success
  integer         :: dst_len, y

  integer*4, save :: dst1(dst_size), dst3(dst_size) 
  integer*8, save :: dst2(dst_size), dst4(dst_size)
  integer*4, save :: compare_dst1(dst_size), compare_dst3(dst_size) 
  integer*8, save :: compare_dst2(dst_size), compare_dst4(dst_size)

  integer         :: length, errcode, abort

! Function definitions
  integer                   :: my_pe, num_pes
  
  x = 1

  call start_pes(0)
  npes = num_pes()
  me   = my_pe()

  do i = 1, SHMEM_BCAST_SYNC_SIZE, 1
    pSync(i) = SHMEM_SYNC_VALUE
  end do

  if(npes .gt. 1) then

    ! Test shmem_fcollect4
    ! Create the output of fcollect4 and save in compare_dst array
    success = 0
    k = 1
    do i = 1, npes, 1
      do j = 1, 4, 1
        compare_dst1(k) = src1(j)
        k = k + 1
      end do
    end do

    do i = 1, dst_size, 1
      dst1(i) = -1
    end do

    call shmem_barrier_all()

    call shmem_fcollect4(dst1, src1, 4, &
      0, 0, npes, &
      pSync)
    
    if(me .eq. 0) then
      do i = 1, npes * 4, 1
        if(dst1(i) .ne. compare_dst1(i)) then
          success = 1
        end if
      end do

      if(success .eq. 1) then
        write(*,*) "Test shmem_fcollect4: Failed"
      else
        write(*,*) "Test shmem_fcollect4: Passed"
      end if
    end if 

    call shmem_barrier_all()

    ! Create the output of fcollect8 and save in compare_dest array
    success = 0
    j = 0
    k = 1
    
    do i = 1, npes, 1
      do j = 1, 4, 1
        compare_dst2(k) = src2(j);
        k = k + 1
      end do
    end do

    do i = 1, npes * 4, 1
      dst2(i) = -1
    end do

    call shmem_barrier_all()

    call shmem_fcollect8(dst2, src2, 4, 0, 0, npes, pSync)

    if(me .eq. 0) then
      do i = 1, npes * 4, 1
        if(dst2(i) .ne. compare_dst2(i)) then
          success = 1
        end if
      end do

      if(success .eq. 1) then
        write(*,*) "Test shmem_fcollect8: Failed"
      else
        write(*,*) "Test shmem_fcollect8: Passed"
      end if
    end if

    ! Test collect32
    success = 0
    ! Decide the length of the destination array
    x = npes / 4
    y = mod(npes, 4)

    if(y .eq. 1) then
      dst_len = x * 10 + 1
    else if(y .eq. 2) then
      dst_len = x * 10 + 3
    else if(y .eq. 3) then
      dst_len = x * 10 + 6
    else
      dst_len = x * 10
    end if

    ! Create the output of collect32 and save in compare_dst array
    k = 1        
    
    do i = 0, npes - 1, 1
      do j = 1, mod(i, 4) + 1, 1
        compare_dst3(k)=  i * 10 + src3(j)
        k = k + 1
      end do
    end do
		
    do i = 1, dst_len, 1
      dst3(i) = -1
    end do

    do i = 1, 4, 1
      src3(i) = me * 10 + src3(i)
    end do
    
    call shmem_barrier_all()

    call shmem_collect4(dst3, src3, mod(me, 4) + 1, &
    	0, 0, npes, &
    	pSync)

    if(me .eq. 0) then
      
      do i = 1, dst_len, 1
        if(dst3(i) .ne. compare_dst3(i)) then
          success = 1
        end if
      end do

      if(success .eq. 1) then
        write (*,*) "Test shmem_collect4: Failed"
      else
        write (*,*) "Test shmem_collect4: Passed"
      end if
    end if 

    call shmem_barrier_all()

    ! Test shmem_collect8
    success = 0
    if(y .eq. 1) then
      dst_len = x * 10 + 1
    else if(y .eq. 2) then
      dst_len = x * 10 + 3
    else if(y .eq. 3) then
      dst_len = x * 10 + 6
    else
      dst_len = x * 10
    end if

    ! Create the output of collect8 and save in compare_dst array
    k = 1

    do i = 0, npes - 1, 1
      do j = 1, mod(i, 4) + 1, 1
        compare_dst4(k) = src4(j)
        k = k + 1
      end do
    end do

    do i = 1, dst_len, 1
      dst4(i) = -1
    end do

    call shmem_barrier_all()

    call shmem_collect8(dst4, src4, mod(me, 4) + 1, &
      0, 0, npes, &
      pSync)

    call shmem_barrier_all()
  
    if(me .eq. 0) then
      do i = 1, dst_len, 1
        if(dst4(i) .ne. compare_dst4(i)) then
          success = 1
        end if
      end do 
      if(success .eq. 1) then
        write (*,*) "Test shmem_collect8: Failed"
      else
        write (*,*) "Test shmem_collect8: Passed"
      end if
    end if
  else
    write (*,*) "Number of PEs must be > 1 to test collects, test skipped"
  end if
end program test_shmem_collects
