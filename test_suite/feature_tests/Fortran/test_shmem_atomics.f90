! (c) 2011 University of Houston System.  All rights reserved. */
! 
! Tests all atomics   
! shmem_int_swap, shmem_float_swap, shmem_long_swap, shmem_double_swap, shmem_longlong_swap,
! shmem_longlong_cswap, shmem_long_cswap, shmem_int_cswap,
! shmem_long_fadd, shmem_int_fadd,  shmem_longlong_fadd,  
! shmem_long_finc, shmem_int_finc, shmem_longlong_finc,
! 
program test_shmem_atomics
  implicit none
  include 'mpp/shmem.fh'

  integer, save             :: success1_p2 
  integer, save             :: success2_p2 
  integer, save             :: success3_p2 
  integer, save             :: success4_p2 
  integer, save             :: success5_p2 

  integer                   :: me, npes

  integer*4,        save    :: target1
  real*4,           save    :: target2
  integer*8,        save    :: target3
  real*8,           save    :: target4

  integer                   :: swapped_val1, new_val1
  real*4                    :: swapped_val2, new_val2
  integer*8                 :: swapped_val3, new_val3
  real*8                    :: swapped_val4, new_val4

  integer, save             :: success
  integer, save             :: success1_p1
  integer, save             :: success2_p1
  integer, save             :: success3_p1
  integer, save             :: success4_p1 

  integer                   :: errcode, abort, length

! Function definitions
  integer                   :: my_pe, num_pes
  integer*4                 :: shmem_int4_swap, shmem_int4_cswap, shmem_int4_fadd, shmem_int4_finc
  integer*8                 :: shmem_int8_swap, shmem_int8_cswap, shmem_int8_fadd, shmem_int8_finc      
  real*4                    :: shmem_real4_swap
  real*8                    :: shmem_real8_swap

  success = 1

  call start_pes(0)
  me = my_pe()
  npes = num_pes()

  call shmem_barrier_all()

  ! Checks if there are atleast 2 executing PEs

  if (npes .gt. 1) then
    length = 1

    target1 = me
    target2 = me
    target3 = me
    target4 = me

    new_val1 = me 
    new_val2 = real(me) 
    new_val3 = me
    new_val4 = real(me)

    success1_p1 = -1 
    success1_p2 = -1 
    success2_p1 = -1 
    success2_p2 = -1 
    success3_p1 = -1 
    success3_p2 = -1 
    success4_p1 = -1 
    success4_p2 = -1 

    call shmem_barrier_all()

    swapped_val1 = shmem_int4_swap(target1, new_val1, mod((me + 1), npes))
    swapped_val2 = shmem_real4_swap(target2, new_val2, mod((me + 1), npes))
    swapped_val3 = shmem_int8_swap(target3, new_val3, mod((me + 1), npes))
    swapped_val4 = shmem_real8_swap(target4, new_val4, mod((me + 1), npes))

    call shmem_barrier_all()

    ! To validate the working of swap we need to check the value received at the PE that initiated the swap 
    !  as well as the target PE
    ! 

    if(me .eq. 0) then
      if(swapped_val1 .eq. 1) then
        success1_p1 = 1
      end if
      if(swapped_val2 .eq. 1) then
        success2_p1 = 1
      end if
      if(swapped_val3 .eq. 1) then
        success3_p1 = 1
      end if
      if(swapped_val4 .eq. 1) then
        success4_p1 = 1
      end if
    end if

    if(me .eq. 1) then
      if(target1 .eq. 0) then
        call shmem_integer_put(success1_p2, success, 1, 0)
      end if
      if(target2 .eq. 0) then
        call shmem_integer_put(success2_p2, success, 1, 0)
      end if
      if(target3 .eq. 0) then
        call shmem_integer_put(success3_p2, success, 1, 0)
      end if
      if(target4 .eq. 0) then
        call shmem_integer_put(success4_p2, success, 1, 0)
      end if
    end if

    call shmem_barrier_all()

    if(me .eq. 0) then
      if(success1_p1 .eq. 1 .and. success1_p2 .eq. 1) then
        write (*,*) "Test shmem_int4_swap: Passed"
      else
        write (*,*) "Test shmem_int4_swap: Failed"
      end if

      if(success2_p1 .eq. 1 .and. success2_p2 .eq. 1) then
        write (*,*) "Test shmem_real4_swap: Passed"   
      else
        write (*,*) "Test shmem_real4_swap: Failed"
      end if

      if(success3_p1 .eq. 1 .and. success3_p2 .eq. 1) then
        write (*,*) "Test shmem_int8_swap: Passed"
      else
        write (*,*) "Test shmem_int8_swap: Failed"
      end if

      if(success4_p1 .eq. 1 .and. success4_p2 .eq. 1) then
        write (*,*) "Test shmem_real8_swap: Passed"
      else
        write (*,*) "Test shmem_real8_swap: Failed"
      end if
    end if

    call shmem_barrier_all()

    ! Test conditional swaps
    ! shmem_longlong_cswap, shmem_long_cswap, shmem_int_cswap,
    ! 

    target1 = me 
    target3 = me

    new_val1 = me
    new_val3 = me 

    success1_p1 = -1 
    success1_p2 = -1 
    success3_p1 = -1 
    success3_p2 = -1 

    if(me .eq. 1) then
      write (*,*) "Value before change: ", target3
    end if
      
    call shmem_barrier_all()

    swapped_val1 = shmem_int4_cswap(target1, me + 1, me, 1)
    swapped_val3 = shmem_int8_cswap(target3, int(me + 1, kind=8), int(me, kind=8), 1)

    call shmem_barrier_all()

    if(me .eq. 1) then
      write (*,*) "Value after change: ", target3
    end if

    ! To validate the working of conditionalswap we need to check the value received at the PE that initiated 
    ! the conditional swap as well as the target PE
    !

    if(me .eq. 0) then
      if(swapped_val1 .eq. 1) then
        success1_p1 = 1
      end if

      if(swapped_val3 .eq. 1) then
        success3_p1 = 1
      end if

    end if

    if(me .eq. 1) then
      if(target1 .eq. 0) then
        call shmem_integer_put(success1_p2, success, 1, 0)
      end if

      if(target3 .eq. 0) then
        call shmem_integer_put(success3_p2, success, 1, 0)
      end if

    end if

    call shmem_barrier_all()

    if(me .eq. 0) then
      if(success1_p1 .eq. 1 .and. success1_p2 .eq. 1) then
        write (*,*) "Test shmem_int4_cswap: Passed"
      else
        write (*,*) "Test shmem_int4_cswap: Failed"
      end if

      if(success3_p1 .eq. 1 .and. success3_p2 .eq. 1) then
        write (*,*) "Test shmem_int8_cswap: Passed"
      else
        write (*,*) "Test shmem_int8_cswap: Failed"
      end if
    end if 

    call shmem_barrier_all()

    ! Test shmem_long_fadd, shmem_int_fadd,  shmem_longlong_fadd 

    target1 = me 
    target3 = me

    new_val1 = me
    new_val3 = me

    success1_p1 = -1  
    success1_p2 = -1 
    success3_p1 = -1 
    success3_p2 = -1 

    call shmem_barrier_all()

    swapped_val1 = shmem_int4_fadd(target1, me, 0)
    swapped_val3 = shmem_int8_fadd(target3, int(me, kind=8), 0)

    call shmem_barrier_all()

    ! To validate the working of fetch and add we need to check the old value received at the PE that initiated 
    ! the fetch and increment as well as the new value on the target PE
    !

    if(me .eq. npes - 1) then
      if(swapped_val1 .eq. 0 ) then
        success1_p1 = 1
      end if

      if(swapped_val3 .eq. 0) then
        success3_p1 = 1
      end if
    end if

    if(me .eq. 0) then
      if(target1 .eq. npes - 1) then
        call shmem_integer_put(success1_p2, success, 1, npes - 1)
      end if

      if(target3 .eq. npes - 1) then
        call shmem_integer_put(success3_p2, success, 1, npes - 1)        
      end if
    end if

    call shmem_barrier_all()

    if(me .eq. npes - 1) then
      if(success1_p1 .eq. 1 .and. success1_p2 .eq. 1) then
        write(*,*) "Test shmem_int4_fadd: Passed" 
      else
        write(*,*) "Test shmem_int4_fadd: Failed"
      end if

      if(success3_p1 .eq. 1 .and. success3_p2 .eq. 1) then
        write (*,*) "Test shmem_int8_fadd: Passed"
      else
        write (*,*) "Test shmem_int8_fadd: Failed"
      end if 

    end if 
    call shmem_barrier_all()

    ! Test shmem_long_finc, shmem_int_finc, shmem_longlong_finc */

    target1 = me
    target3 = me

    new_val1 = me
    new_val3 = me

    success1_p1 = -1 
    success1_p2 = -1
    success3_p1 = -1
    success3_p2 = -1

    call shmem_barrier_all()

    swapped_val1 = shmem_int4_finc(target1, mod((me + 1), npes))
    swapped_val3 = shmem_int8_finc(target3, mod((me + 1), npes))

    call shmem_barrier_all()

    ! To validate the working of fetch and increment we need to check the old value received at the PE that initiated 
    ! the fetch and increment as well as the new value on the target PE
    ! 

    if(me .eq. npes - 1) then
      if(swapped_val1 .eq. 0 ) then
        success1_p1 = 1
      end if

      if(swapped_val3 .eq. 0) then
        success3_p1 = 1
      end if

    end if

    if(me .eq. 0) then
      if(target1 .eq. 1) then
        call shmem_integer_put(success1_p2, success, 1, npes - 1)
      end if

      if(target3 .eq. 1) then
        call shmem_integer_put(success3_p2, success, 1, npes - 1)
      end if

    end if

    call shmem_barrier_all()

    if(me .eq. npes - 1) then
      if(success1_p1 .eq. 1 .and. success1_p2 .eq. 1) then
        write (*,*) "Test shmem_int4_finc: Passed"
      else
        write (*,*) "Test shmem_int4_finc: Failed"
      end if

      if(success3_p1 .eq. 1 .and. success3_p2 .eq. 1 ) then
        write (*,*) "Test shmem_int8_finc: Passed" 
      else
        write (*,*) "Test shmem_int8_finc: Failed"
      end if

    end if

    call shmem_barrier_all()

  else
    write (*,*) "Number of PEs must be > 1 to test shmem atomics, test skipped"
  end if 

end program test_shmem_atomics
