! (c) 2011 University of Houston System.  All rights reserved. */
! 
! shmem_int4_wait,  shmem_int4_wait_until,  shmem_int8_wait,
! shmem_int8_wait_until
! 
! Tests conditational wait (shmem_int8_wait) call
! PE 1 waits for PE 0 to send something other than 9.
! Send 4 9s to test wait condition, then some random values until != 9.
! 
program test_shmem_synchronization
  implicit none
  include 'mpp/shmem.fh'

  integer         :: me, npes
  integer*8, save :: dest
  integer*8       :: src
  integer         :: i

! Function definitions
  integer    :: my_pe, num_pes

  src = 9

  call start_pes(0)
  me   = my_pe()
  npes = num_pes()
  
  if(npes .gt. 1) then
  
    dest = 9
  
    call shmem_barrier_all()
  
    if (me .eq. 0) then
      do i = 1, 4, 1
        call shmem_integer_put(dest, src, 1, 1);
      end do
  
      do i = 1, 10, 1
        src = get_random_number()
        call shmem_integer_put(dest, src, 1, 1)
        if (src .ne. 9) then
          exit
        end if
      end do
    end if
  
    call shmem_barrier_all()
  
    if (me .eq. 1) then
      call shmem_int8_wait(dest, 9)
      write(*,*) "Test for conditional wait: Passed"
    end if
  
    call shmem_barrier_all()
    
    dest = 9
    call shmem_barrier_all()
  
    if (me .eq. 0) then
      do i = 1, 4, 1
        src = 9
        call shmem_integer_put(dest, src, 1, 1)
      end do
  
      do i = 1, 10, 1
        src = mod(get_random_number(), 10)
        call shmem_integer_put(dest, src, 1, 1)
        if (src .ne. 9) then
          exit
        end if
      end do 
    end if
  
    call shmem_barrier_all()
  
    if (me .eq. 1) then
      call shmem_int8_wait_until(dest, SHMEM_CMP_NE, 9)
      write (*,*) "Test for explicit conditional wait: Passed"
    end if
  
    call shmem_barrier_all()
    
  else
    write(*,*) "Test for conditional wait requires more than 1 PE, test skipped"
  end if
contains
  integer function get_random_number()
    implicit none
    real :: numbers(3)
    integer :: a_number

    call init_random_seed()
    call random_number(numbers)

    a_number = int(numbers(1) * 100)

    get_random_number = a_number

  end function get_random_number

  subroutine init_random_seed()
    implicit none
    integer :: i, n, clock
    integer, dimension(:), allocatable :: seed

    call random_seed(size = n)
    allocate(seed(n))

    call system_clock(count=clock)

    seed = clock + 37 * (/ (i - 1, i = 1, n) /)
    call random_seed(put = seed)

    deallocate(seed)
  end subroutine

end program test_shmem_synchronization
