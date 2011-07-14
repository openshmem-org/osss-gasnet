! (c) 2011 University of Houston System.  All rights reserved.


!
! integer value put to global
!
!

program iip
  include 'mpp/shmem.fh'

  integer, save :: n
  integer :: me

  call start_pes(0)
  me = my_pe()

  n = 3;

  call shmem_barrier_all()

  if (me == 0) then
     call shmem_integer_put(n, 42, 1, 1)
  end if

  call shmem_barrier_all()

  ! now check

  if (me == 1) then
     if (n == 42) then
        print *, "OK"
     else
        print *, "FAIL"
     end if
  end if

end program iip
