! (c) 2011 University of Houston System.  All rights reserved.


!
! expected output on 2 PEs:
!
! 0: old = 22, dst = 22
! 1: old = -1, dst = 23
!

program finc
  integer, save :: dst
  integer me
  integer old

  include 'shmem.fh'

  call start_pes(0)
  me = my_pe()

  old = -1
  dst = 22
  call shmem_barrier_all()

  if (me == 0) then
    old = shmem_int4_finc(dst, 1)
  end if
  call shmem_barrier_all()

  print *, me, ': old = ', old, ', dst = ', dst

end program finc
