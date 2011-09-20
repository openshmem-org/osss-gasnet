! (c) 2011 University of Houston System.  All rights reserved. */
!
! Tests shmem_set_lock, shmem_test_lock
! and shmem_clear_lock calls*/

program test_shmem_lock
  implicit none
  include 'mpp/shmem.fh'

  integer*8, save :: L
  integer  , save :: x

  integer         :: me, npes;
  integer         :: slp;
  integer         :: ret_val;
  integer         :: new_val;

  ! Function definitions
  integer                   :: my_pe, num_pes
  integer                   :: shmem_test_lock

  call start_pes(0)
  me   = my_pe()
  npes = num_pes()
  L = 0
  x = 0
  ret_val = -1

  if(npes .gt. 1) then

    slp = 1
    call shmem_barrier_all()

    call shmem_set_lock(L)

    call shmem_integer_get(new_val, x, 1, 0)
    new_val = new_val + 1

    call shmem_integer_put(x, new_val, 1, 0)  ! increment x on PE 0
    call shmem_quiet() 

    call shmem_clear_lock(L)

    call shmem_barrier_all()
 
    if(me .eq. 0) then
      if(x .eq. npes) then
        write(*,*) "Test for set, and clear lock: Passed"
      else
        write(*,*) "Test for set, and clear lock: Failed"
      end if
      x=0
    end if

    call shmem_barrier_all()

    ret_val = 1
    do while(ret_val .eq. 1)
      ret_val = shmem_test_lock(L)
    end do

    call shmem_integer_get(new_val, x, 1, 0)
    new_val = new_val + 1

    call shmem_integer_put(x, new_val, 1, 0)  ! increment x on PE 0 

    call shmem_quiet()

    call shmem_clear_lock(L)

    call shmem_barrier_all()

    if(me .eq. 0) then
      if(x .eq. npes) then
        write(*,*) "Test for test lock: Passed"
      else
        write(*,*) "Test for test lock: Failed"
      end if

    end if 

    call shmem_barrier_all()
  else
    write(*,*) "Number of PEs must be > 1 to test locks, test skipped"
  end if
end program test_shmem_lock
