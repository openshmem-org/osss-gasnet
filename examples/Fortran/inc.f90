! (c) 2011 University of Houston System.  All rights reserved.


!
! expected output on 2 PEs:
!
! 0: dst = 74
! 1: dst = 75
!

program inc
  integer me
  integer, save :: dst

  include 'shmem.fh'

  call start_pes(0)
  me = my_pe()

  dst = 74
  call shmem_barrier_all()

  if (me == 0) then
     call shmem_int4_inc(dst, 1)
  end if
  call shmem_barrier_all()

  if (me == 1) then
     if (dst == 75) then
        print *, 'OK'
     else
        print *, 'FAIL'
     end if
  end if

end program inc
