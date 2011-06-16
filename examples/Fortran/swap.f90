! (c) 2011 University of Houston.  All rights reserved.


!
!
! swap values between odd numbered PEs and their right (modulo)
! neighbor.  Show result of swap.
!

program swap

  include 'mpp/shmem.fh'
  integer me, npes
  integer, save :: target
  integer swapped_val, new_val

  call start_pes(0)
  me = my_pe()
  npes = num_pes()

  target = me
  call shmem_barrier_all()

  new_val = me

  if (mod(me, 2) == 1) then
    swapped_val = shmem_swap(target, new_val, mod((me + 1), npes))
    print *, me, ': target = ', target, ' swapped = ' , swapped_val
  end if

end program swap
