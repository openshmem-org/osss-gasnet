! (c) 2011 University of Houston System.  All rights reserved.


!
! expected output on 2 PEs:
!
! 0: old = -1, dst = 66
! 1: old = 22, dst = 22
!

program fadd

  include 'shmem.fh'

  integer, save :: dst
  integer me, old

  call start_pes(0)
  me = my_pe()

  old = -1;
  dst = 22;
  call shmem_barrier_all()

  if (me == 1) then
     old = shmem_int4_fadd(dst, 44, 0);
  end if
  call shmem_barrier_all()

  if (me == 0) then
     if (old == (-1) .and. dst == 66) then
        print *, 'OK'
     else
        print *, 'FAIL'
     end if
  end if

end program fadd
