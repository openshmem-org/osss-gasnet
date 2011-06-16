! (c) 2011 University of Houston.  All rights reserved.


!
! test save and common block variables are recognized as symmetric
!

program comm
  integer :: re, im
  common /complex/ re, im
  integer, save :: x, y

  call start_pes(0)
  me = my_pe()

  if (me == 0) then
     call shmem_integer_put(re, im, 1, 1)
     call shmem_integer_put(x, y, 1, 1)
  end if

  call shmem_barrier_all

end program
