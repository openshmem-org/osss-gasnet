! (c) 2011 University of Houston System.  All rights reserved. */
! 
! 
! 
! Calls tested
! shmem_short_get, shmem_int_get, shmem_long_get, shmem_longdouble_get,
! shmem_longlong_get, shmem_double_get, shmem_float_get,
! TODO:shmem_complexf_get, shmem_complexd_get
! shmem_getmem, shmem_get32, shmem_get64, shmem_get128
! 
! 
! All PEs get an array from right neighbor
!  
program test_shmem_get
  implicit none
  include 'mpp/shmem.fh'

  integer, parameter :: N = 7

  integer          ::  i,j
  integer          ::  nextpe
  integer          ::  me, npes
  integer          ::  success1,success2,success3, success4, success5, success6

  integer          :: dest1(N)
  real             :: dest2(N)
  double precision :: dest3(N)
  character        :: dest4(N)
  character        :: dest5(N)
  logical          :: dest6(N)

  integer          , save :: src1(N)
  real             , save :: src2(N)
  double precision , save :: src3(N)
  character        , save :: src4(N)
  character        , save :: src5(N)
  logical          , save :: src6(N)

  integer          :: length, errcode, abort

! Function definitions
  integer                   :: my_pe, num_pes

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

    call shmem_integer_get(dest1, src1, N, nextpe)
    call shmem_real_get(dest2, src2, N, nextpe)
    call shmem_double_get(dest3, src3, N, nextpe)
    call shmem_character_get(dest4, src4, N, nextpe)
    call shmem_getmem(dest5, src5, N, nextpe)
    call shmem_logical_get(dest6, src6, N, nextpe)

    call shmem_barrier_all()

    if(me .eq. 0) then
      do i = 1, N, 1
        if(dest1(i) .ne. 1) then
          success1 = success1 + 1
        end if
        if(dest2(i) .ne. 1) then
          success2 = success2 + 1
        end if
        if(dest3(i) .ne. 1) then
          success3 = success3 + 1
        end if
        if(dest4(i) .ne. char(1)) then
          success4 = success4 + 1
        end if
        if(dest5(i) .ne. char(1)) then
          success5 = success5 + 1
        end if
        if(.not. dest6(i)) then
          success6 = success6 + 1
        end if
      end do 

      if(success1 .eq. 0) then
        write(*,*) "Test shmem_integer_get: Passed" 
      else
        write(*,*) "Test shmem_integer_get: Failed"
      end if
      if(success2 .eq. 0) then
        write(*,*) "Test shmem_real_get: Passed"  
      else
        write(*,*) "Test shmem_real_get: Failed"
      end if
      if(success3 .eq. 0) then
        write(*,*) "Test shmem_double_get: Passed"  
      else
        write(*,*) "Test shmem_double_get: Failed"
      end if
      if(success4 .eq. 0) then
        write(*,*) "Test shmem_character_get: Passed"  
      else
        write(*,*) "Test shmem_character_get: Failed"
      end if
      if(success5 .eq. 0) then
        write(*,*) "Test shmem_getmem: Passed"  
      else
        write(*,*) "Test shmem_getmem: Failed"
      end if
      if(success6 .eq. 0) then
        write(*,*) "Test shmem_logical_get: Passed"  
      else
        write(*,*) "Test shmem_logical_get: Failed"
      end if
    end if 

    call shmem_barrier_all()

    ! Testing shmem_get32, shmem_get64, shmem_get128 
    if(2 .eq. 4) then
      do i = 1, N, 1
        dest1(i) = -9
      end do 

      success1 = 0

      call shmem_barrier_all()

      call shmem_get32(dest1, src1, N, nextpe)
      call shmem_get64(dest3, src3, N, nextpe)
      call shmem_get128(dest4, src4, N, nextpe)

      call shmem_barrier_all()

      if(me .eq. 0) then
        do i = 1, N, 1
          if(dest2(i) .ne. 1) then
            success2 = 1
          end if
          if(dest3(i) .ne. 1) then
            success3 = 1
          end if
          if(dest4(i) .ne. char(9)) then
            success4 = 1
          end if
        end do

        if(success2 .eq. 0) then
          write(*,*) "Test shmem_get32: Passed"  
        else
          write(*,*) "Test shmem_get32: Failed"
        end if

        if(success3 .eq. 0) then
          write(*,*) "Test shmem_get64: Passed" 
        else
          write(*,*) "Test shmem_get64: Failed"
        end if

        if(success4 .eq. 0) then
          write(*,*) "Test shmem_get128: Passed"  
        else
          write(*,*) "Test shmem_get128: Failed"
        end if
      end if
    end if

    call shmem_barrier_all()

  else
    write(*,*) "Number of PEs must be > 1 to test shmem get, test skipped"
  end if
end program
