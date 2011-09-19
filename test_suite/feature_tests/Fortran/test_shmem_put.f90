! (c) 2011 University of Houston System.  All rights reserved. */
! 
! 
! 
! Calls tested
! shmem_short_put, shmem_int_put, shmem_long_put, shmem_longdouble_put,
! shmem_longlong_put, shmem_double_put, shmem_float_put,
! TODO:shmem_complexf_put, shmem_complexd_put
! shmem_putmem, shmem_put32, shmem_put64, shmem_put128
! 
! 
! All PEs put an array on the right neighbor 
!  
program test_shmem_put
  implicit none
  include 'mpp/shmem.fh'

  integer, parameter :: N = 7

  integer                 ::  i,j
  integer                 ::  nextpe
  integer                 ::  me, npes
  integer                 ::  success1,success2,success3, success4, success5, success6

  integer          , save :: dest1(N)
  real             , save :: dest2(N)
  double precision , save :: dest3(N)
  character        , save :: dest4(N)
  character        , save :: dest5(N)
  logical          , save :: dest6(N)

  integer                 :: src1(N)
  real                    :: src2(N)
  double precision        :: src3(N)
  character               :: src4(N)
  character               :: src5(N)
  logical                 :: src6(N)

  integer                 :: length, errcode, abort

  call start_pes(0)
  me   = my_pe();
  npes = num_pes();

  if(npes .gt. 1) then

    success1 = 0
    success2 = 0
    success3 = 0
    success4 = 0
    success5 = 0
    success6 = 0

    length = N

    do i = 1, N, 1
      dest1(i) = -9
      dest2(i) = real(9)
      dest3(i) = dble(9)
      dest4(i) = char(9)
      dest5(i) = char(9)      
      dest6(i) = .false.
    end do 

!   call shpalloc(src1, N, errcode, abort)
!   call shpalloc(src2, N, errcode, abort)
!   call shpalloc(src3, N, errcode, abort)
!   call shpalloc(src4, N, errcode, abort)
!   call shpalloc(src5, N, errcode, abort)
!   call shpalloc(src6, N, errcode, abort)

    do i = 1, N, 1
      src1(i) = me
      src2(i) = real(me)
      src3(i) = dble(me)
      src4(i) = char(me)
      src5(i) = char(me)
      src6(i) = .true.
    end do 

    nextpe = mod((me + 1), npes)

    call shmem_barrier_all()

    call shmem_integer_put(dest1, src1, N, nextpe)
    call shmem_real_put(dest2, src2, N, nextpe)
    call shmem_double_put(dest3, src3, N, nextpe)
    call shmem_character_put(dest4, src4, N, nextpe)
    call shmem_putmem(dest5, src5, N, nextpe)
    call shmem_logical_put(dest6, src6, N, nextpe)

    call shmem_barrier_all()

    if(me .eq. 0) then
      do i = 1, N, 1
        if(dest1(i) .ne. (npes - 1)) then
          success1 = success1 + 1
        end if
        if(dest2(i) .ne. (npes - 1)) then
          success2 = success2 + 1
        end if
        if(dest3(i) .ne. (npes - 1)) then
          success3 = success3 + 1
        end if
        if(dest4(i) .ne. char(npes - 1)) then
          success4 = success4 + 1
        end if
        if(dest5(i) .ne. char(npes - 1)) then
          success5 = success5 + 1
        end if
        if(.not. dest6(i)) then
          success6 = success6 + 1
        end if
      end do 

      if(success1 .eq. 0) then
        write(*,*) "Test shmem_integer_put: Passed" 
      else
        write(*,*) "Test shmem_integer_put: Failed"
      end if
      if(success2 .eq. 0) then
        write(*,*) "Test shmem_real_put: Passed"  
      else
        write(*,*) "Test shmem_real_put: Failed"
      end if
      if(success3 .eq. 0) then
        write(*,*) "Test shmem_double_put: Passed"  
      else
        write(*,*) "Test shmem_double_put: Failed"
      end if
      if(success4 .eq. 0) then
        write(*,*) "Test shmem_character_put: Passed"  
      else
        write(*,*) "Test shmem_character_put: Failed"
      end if
      if(success5 .eq. 0) then
        write(*,*) "Test shmem_putmem: Passed"  
      else
        write(*,*) "Test shmem_putmem: Failed"
      end if
      if(success6 .eq. 0) then
        write(*,*) "Test shmem_logical_put: Passed"  
      else
        write(*,*) "Test shmem_logical_put: Failed"
      end if
    end if 

    call shmem_barrier_all()

    ! Testing shmem_put32, shmem_put64, shmem_put128 
    if(2 .eq. 4) then
      do i = 1, N, 1
        dest1(i) = -9
      end do 

      success1 = 0

      call shmem_barrier_all()

      call shmem_put32(dest1, src1, N, nextpe)
      call shmem_put64(dest3, src3, N, nextpe)
      call shmem_put128(dest4, src4, N, nextpe)

      call shmem_barrier_all()

      if(me .eq. 0) then
        do i = 1, N, 1
          if(dest2(i) .ne. npes - 1) then
            success2 = success2 + 1
          end if
          if(dest3(i) .ne. npes - 1) then
            success3 = success3 + 1
          end if
          if(dest4(i) .ne. char(npes - 1)) then
            success4 = success4 + 1
          end if
        end do

        if(success2 .eq. 0) then
          write(*,*) "Test shmem_put32: Passed"  
        else
          write(*,*) "Test shmem_put32: Failed"
        end if

        if(success3 .eq. 0) then
          write(*,*) "Test shmem_put64: Passed" 
        else
          write(*,*) "Test shmem_put64: Failed"
        end if

        if(success4 .eq. 0) then
          write(*,*) "Test shmem_put128: Passed"  
        else
          write(*,*) "Test shmem_put128: Failed"
        end if
      end if
    end if

    call shmem_barrier_all()

  else
    write(*,*) "Number of PEs must be > 1 to test shmem get, test skipped"
  end if
end program
