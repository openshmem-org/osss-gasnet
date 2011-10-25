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
  real     , save      :: src2(N)
  real     , save      :: dst2(N)
  real*8   , save      :: dst3(N)
  real*8   , save      :: src3(N)
  integer  , save      :: src4(N) 
  integer  , save      :: dst4(N)
  integer*8, save      :: src5
  integer*8, save      :: dst5
  real     , save      :: src6                
  real     , save      :: dst6
  real*8   , save      :: src7
  real*8   , save      :: dst7
  integer  , save      :: expected_result
  integer  , save      :: pWrk1(SHMEM_REDUCE_SYNC_SIZE)
  integer*8, save      :: pWrk2(SHMEM_REDUCE_SYNC_SIZE)

  integer              :: i,j
  integer              :: me, npes
  integer              :: success
  integer              :: success1
  integer              :: success2
  integer              :: success3

  ! Function definitions
  integer                   :: my_pe, num_pes

  success = 0
  success1 = 0
  success2 = 0
  success3 = 0

  call start_pes(0)
  me   = my_pe()
  npes = num_pes()

  do i = 1, SHMEM_BCAST_SYNC_SIZE, 1
    pSync(i) = SHMEM_SYNC_VALUE
  end do

  do i = 1, N, 1
    src(i) = me + i
    src4(i) = me + i
    src2(i) = me + i
    src3(i) = me + i
  end do
  
  ! Test MAX
  call shmem_barrier_all()

  call shmem_int8_max_to_all(dst, src, N, 0, 0, npes, pWrk2, pSync)
  call shmem_int4_max_to_all(dst4, src4, N, 0, 0, npes, pWrk2, pSync)
  call shmem_real4_max_to_all(dst2, src2, N, 0, 0, npes, pWrk2, pSync)
  call shmem_real8_max_to_all(dst3, src3, N, 0, 0, npes, pWrk2, pSync)

  if(me .eq. 0) then
    j = 0
    do i = 1, N, 1
      if(dst(i) .ne. npes + j) then
        success = 1
      end if
      if(dst4(i) .ne. npes + j) then
        success1 = 1
      end if
      if(dst2(i) .ne. npes + j) then
        success2 = 1
      end if
      if(dst3(i) .ne. npes + j) then
        success3 = 1
      end if
      j = j + 1 
    end do

    if(success .eq. 1) then
      write(*,*) "Test Reduction operation (int8_max): Failed"
    else
      write(*,*) "Test Reduction operation (int8_max): Passed"
    end if
    if(success1 .eq. 1) then
      write(*,*) "Test Reduction operation (int4_max): Failed"
    else
      write(*,*) "Test Reduction operation (int4_max): Passed"
    end if
    if(success2 .eq. 1) then
      write(*,*) "Test Reduction operation (real4_max): Failed"
    else
      write(*,*) "Test Reduction operation (real4_max): Passed"
    end if
    if(success3 .eq. 1) then
      write(*,*) "Test Reduction operation (real8_max): Failed"
    else
      write(*,*) "Test Reduction operation (real8_max): Passed"
    end if
  end if
  
  
  ! Test MIN
  success = 0
  success1 = 0
  success2 = 0
  success3 = 0

  do i = 1, N, 1
    dst(i) = -9
    dst2(i) = -9
    dst3(i) = -9
    dst4(i) = -9
  end do 
   
  call shmem_barrier_all()
  
  call shmem_int8_min_to_all(dst, src, N, 0, 0, npes, pWrk2, pSync)
  call shmem_int4_min_to_all(dst4, src4, N, 0, 0, npes, pWrk2, pSync)
  call shmem_real4_min_to_all(dst2, src2, N, 0, 0, npes, pWrk2, pSync)
  call shmem_real8_min_to_all(dst3, src3, N, 0, 0, npes, pWrk2, pSync)

  if(me .eq. 0) then
    do i = 1, N, 1
      if(dst(i) .ne. i) then
        success = 1
      end if
      if(dst4(i) .ne. i) then
        success1 = 1
      end if
      if(dst2(i) .ne. i) then
        success2 = 1
      end if
      if(dst3(i) .ne. i) then
        success3 = 1
      end if
    end do

    if(success .eq. 1) then
      write(*,*) "Test Reduction operation (int8_min): Failed"
    else
      write(*,*) "Test Reduction operation (int8_min): Passed"
    end if
    if(success1 .eq. 1) then
      write(*,*) "Test Reduction operation (int4_min): Failed"
    else
      write(*,*) "Test Reduction operation (int4_min): Passed"
    end if
    if(success2 .eq. 1) then
      write(*,*) "Test Reduction operation (real4_min): Failed"
    else
      write(*,*) "Test Reduction operation (real4_min): Passed"
    end if
    if(success3 .eq. 1) then
      write(*,*) "Test Reduction operation (real8_min): Failed"
    else
      write(*,*) "Test Reduction operation (real8_min): Passed"
    end if

  end if
  
  ! Test SUM
  success = 0
  success1 = 0
  success2 = 0
  success3 = 0

  src1 = me + 1
  src5 = me + 1
  src6 = me + 1
  src7 = me + 1

  dst1 = -9
  dst5 = -9
  dst6 = -9
  dst7 = -9

  call shmem_barrier_all()

  call shmem_int4_sum_to_all(dst1, src1, 1, 0, 0, npes, pWrk1, pSync)
  call shmem_int8_sum_to_all(dst5, src5, 1, 0, 0, npes, pWrk1, pSync)
  call shmem_real4_sum_to_all(dst6, src6, 1, 0, 0, npes, pWrk1, pSync)
  call shmem_real8_sum_to_all(dst7, src7, 1, 0, 0, npes, pWrk1, pSync)

  if(me .eq. 0) then
    if(dst1 .ne. (npes * (npes+1)/2)) then
      write(*,*) "Test Reduction operation (int4_sum): Failed"
    else
      write(*,*) "Test Reduction operation (int4_sum): Passed"
    end if
    if(dst5 .ne. (npes * (npes+1)/2)) then
      write(*,*) "Test Reduction operation (int8_sum): Failed"
    else
      write(*,*) "Test Reduction operation (int8_sum): Passed"
    end if
    if(dst6 .ne. (npes * (npes+1)/2)) then
      write(*,*) "Test Reduction operation (real4_sum): Failed"
    else
      write(*,*) "Test Reduction operation (real4_sum): Passed"
    end if
    if(dst7 .ne. (npes * (npes+1)/2)) then
      write(*,*) "Test Reduction operation (real8_sum): Failed"
    else
      write(*,*) "Test Reduction operation (real8_sum): Passed"
    end if
  end if
  
  ! Test AND 
  success = 0
  src1 = mod(me, 3) + 1
  src5 = mod(me, 3) + 1
  dst1 = -9
  dst5 = -9
  call shmem_barrier_all()
  
  call shmem_int4_and_to_all(dst1, src1, 1, 0, 0, npes, pWrk1, pSync)
  call shmem_int8_and_to_all(dst5, src5, 1, 0, 0, npes, pWrk1, pSync)
  
  if(me .eq. 0) then
    if(dst1 .ne. 0) then
      write(*,*) "Test Reduction operation (int4_and): Failed"
    else
      write(*,*) "Test Reduction operation (int4_and): Passed"
    end if
    if(dst5 .ne. 0) then
      write(*,*) "Test Reduction operation (int8_and): Failed"
    else
      write(*,*) "Test Reduction operation (int8_and): Passed"
    end if
  end if
  
  ! Test PROD
  src(:) = (me + 1)
  src2(:) = (me + 1)
  src3(:) = (me + 1)
  src4(:) = (me + 1)
  dst(:) = -9
  dst2(:) = -9
  dst3(:) = -9
  dst4(:) = -9

  success = 0
  success1 = 0
  success2 = 0
  success3 = 0

  expected_result = 1

  do i = 1, npes, 1
    expected_result = expected_result * i
  end do 
   
  call shmem_barrier_all()
 
  call shmem_int4_prod_to_all(dst4, src4, N, 0, 0, npes, pWrk1, pSync)
  call shmem_int8_prod_to_all(dst, src, N, 0, 0, npes, pWrk1, pSync)
  call shmem_real4_prod_to_all(dst2, src2, N, 0, 0, npes, pWrk1, pSync)
  call shmem_real8_prod_to_all(dst3, src3, N, 0, 0, npes, pWrk1, pSync)
 
  if(me .eq. 0) then
    do i = 1, N, 1 
      if(dst4(i) .ne. expected_result) then
        success = 1
      end if
      if(dst(i) .ne. expected_result) then
        success1 = 1
      end if
      if(dst2(i) .ne. expected_result) then
        success2 = 1
      end if
      if(dst3(i) .ne. expected_result) then
        success3 = 1
      end if
    end do
    
    if(success .ne. 0) then
      write(*,*) "Test Reduction operation (int4_prod): Failed"
    else
      write(*,*) "Test Reduction operation (int4_prod): Passed"
    end if
    if(success1 .ne. 0) then
      write(*,*) "Test Reduction operation (int8_prod): Failed"
    else
      write(*,*) "Test Reduction operation (int8_prod): Passed"
    end if
    if(success2 .ne. 0) then
      write(*,*) "Test Reduction operation (real4_prod): Failed"
    else
      write(*,*) "Test Reduction operation (real4_prod): Passed"
    end if
    if(success3 .ne. 0) then
      write(*,*) "Test Reduction operation (real8_prod): Failed"
    else
      write(*,*) "Test Reduction operation (real8_prod): Passed"
    end if
  end if
 
  ! Test OR
  src(:) =  mod((me + 1), 4) 
  src2(:) = mod((me + 1), 4)
  dst(:) = -9
  dst2(:) = -9

  success = 0
  success1 = 0

  expected_result = 3

  call shmem_barrier_all()
 
  call shmem_int4_or_to_all(dst4, src4, N, 0, 0, npes, pWrk1, pSync)
  call shmem_int8_or_to_all(dst, src, N, 0, 0, npes, pWrk1, pSync)
 
  if(me .eq. 0) then
    do i = 1, N, 1 
      if(dst4(i) .ne. expected_result) then
        success = 1
      end if
      if(dst(i) .ne. expected_result) then
        success1 = 1
      end if
    end do
    
    if(success .ne. 0) then
      write(*,*) "Test Reduction operation (int4_or): Failed"
    else
      write(*,*) "Test Reduction operation (int4_or): Passed"
    end if
    if(success1 .ne. 0) then
      write(*,*) "Test Reduction operation (int8_or): Failed"
    else
      write(*,*) "Test Reduction operation (int8_or): Passed"
    end if
  end if
 
  ! Test XOR
  src(:) =  mod(me , 2) 
  src4(:) = mod(me , 2)
  dst(:) = -9
  dst4(:) = -9
  success = 0
  success1 = 0

  expected_result = mod((npes/2), 2) 

  call shmem_barrier_all()
 
  call shmem_int4_xor_to_all(dst4, src4, N, 0, 0, npes, pWrk1, pSync)
  call shmem_int8_xor_to_all(dst, src, N, 0, 0, npes, pWrk1, pSync)
  
  if(me .eq. 0) then
    do i = 1, N, 1 
      if(dst4(i) .ne. expected_result) then
        success = 1
      end if
      if(dst(i) .ne. expected_result) then
        success1 = 1
      end if
    end do
    
    if(success .ne. 0) then
      write(*,*) "Test Reduction operation (int4_xor): Failed"
    else
      write(*,*) "Test Reduction operation (int4_xor): Passed"
    end if
    if(success1 .ne. 0) then
      write(*,*) "Test Reduction operation (int8_xor): Failed"
    else
      write(*,*) "Test Reduction operation (int8_xor): Passed"
    end if
  end if
end program test_shmem_reduction
