! (c) 2011 University of Houston System.  All rights reserved. */
! 
! 
! 
! Tests shmem_int8_max_to_all, shmem_int8_min_to_all, shmem_int4_sum_to_all
! shmem_int4_and_to_all, shmem_int4_prod_to_all, shmem_int4_or_to_all,
! shmem_int4_xor_to_all
! 

program test_shmem_reduction
  implicit none
  include 'mpp/shmem.fh'

  integer,   parameter :: N = 3

  integer,   save      :: pSync(SHMEM_BCAST_SYNC_SIZE)
  integer*8, save      :: src(N)
  integer*8, save      :: dst(N)
  integer  , save      :: src1
  integer  , save      :: dst1
  integer  , save      :: expected_result
  integer  , save      :: pWrk1(SHMEM_REDUCE_SYNC_SIZE)
  integer*8, save      :: pWrk2(SHMEM_REDUCE_SYNC_SIZE)

  integer              :: i,j
  integer              :: me, npes
  integer              :: success

  success = 0

  call start_pes(0)
  me   = my_pe()
  npes = num_pes()

  if (npes .eq. N) then
    do i = 1, SHMEM_BCAST_SYNC_SIZE, 1
      pSync(i) = SHMEM_SYNC_VALUE
    end do

    do i = 1, N, 1
      src(i) = me + i
    end do
    
    ! Test MAX
    call shmem_barrier_all()

    call shmem_int8_max_to_all(dst, src, N, 0, 0, npes, pWrk2, pSync)

    if(me .eq. 0) then
      j = 0
      do i = 1, N, 1
        if(dst(i) .ne. npes + j) then
          success = 1
        end if
        j = j + 1 
      end do

      if(success .eq. 1) then
        write(*,*) "Test Reduction operation (max): Failed"
      else
        write(*,*) "Test Reduction operation (max): Passed"
      end if
    end if
    
    
    ! Test MIN
    success = 0
    do i = 1, N, 1
      dst(i) = -9
    end do 
     
    call shmem_barrier_all()
    
    call shmem_int8_min_to_all(dst, src, N, 0, 0, npes, pWrk2, pSync)

    if(me .eq. 0) then
      do i = 1, N, 1
        if(dst(i) .ne. i) then
          success = 1
        end if
      end do 

      if(success .eq. 1) then
        write(*,*) "Test Reduction operation (min): Failed"
      else
        write(*,*) "Test Reduction operation (min): Passed"
      end if
    end if
    
    ! Test SUM
    success = 0
    src1 = me + 1
    dst1 = -9

    call shmem_barrier_all()

    call shmem_int4_sum_to_all(dst1, src1, 1, 0, 0, npes, pWrk1, pSync)

    if(me .eq. 0) then
      if(dst1 .ne. (npes * (npes+1)/2)) then
        write(*,*) "Test Reduction operation (sum): Failed"
      else
        write(*,*) "Test Reduction operation (sum): Passed"
      end if
    end if
    
    ! Test AND 
    success = 0
    src1 = mod(me, 3) + 1
    dst1 = -9
    call shmem_barrier_all()
    
    call shmem_int4_and_to_all(dst1, src1, 1, 0, 0, npes, pWrk1, pSync)
    
    if(me .eq. 0) then
      if(dst1 .ne. 0) then
        write(*,*) "Test Reduction operation (and): Failed"
      else
        write(*,*) "Test Reduction operation (and): Passed"
      end if
    end if
    
    ! Test PROD
    src1 = (me + 1)
    dst1 = -9
    success = 0
    expected_result = 1

    do i = 1, npes, 1
      expected_result = expected_result * i
    end do 
     
    call shmem_barrier_all()
 
    call shmem_int4_prod_to_all(dst1, src1, 1, 0, 0, npes, pWrk1, pSync)
 
    if(me .eq. 0) then
      if(dst1 .ne. expected_result) then
        write(*,*) "Test Reduction operation (prod): Failed"
      else
        write(*,*) "Test Reduction operation (prod): Passed"
      end if
    end if
 
    ! Test OR
    src1 = mod((me + 1), 4)
    dst1 = -9
    success = 0
    call shmem_barrier_all()
 
    call shmem_int4_or_to_all(dst1, src1, 1, 0, 0, npes, pWrk1, pSync)
 
    if(me .eq. 0) then
      if(dst1 .ne. 3) then
        write(*,*) "Test Reduction operation (or): Failed"
      else
        write(*,*) "Test Reduction operation (or): Passed"
      end if
    end if
 
    ! Test XOR
    src1 = mod(me, 2)
    dst1 = -9
    success = 0

    if(mod(npes, 2) .eq. 0) then
      expected_result = 0
    else
      expected_result = 1
    end if

    call shmem_barrier_all()
 
    call shmem_int4_xor_to_all(dst1, src1, 1, 0, 0, npes, pWrk1, pSync)
    
    if(me .eq. 0) then
      if(dst1 .ne. expected_result) then
        write(*,*) "Test Reduction operation (xor): Failed"
      else
        write(*,*) "Test Reduction operation (xor): Passed"
      end if
    end if
  else
    if(me .eq. 0) then
      write (*,*) "This test must be executed with 3 PEs."
    end if
  end if
end program test_shmem_reduction
