! (c) 2011 University of Houston System.  All rights reserved.


!
! float value put to shmalloc'ed variable
!
program fip
  real e
  parameter ( e = 2.71828182 )
  real epsilon
  parameter ( epsilon = 0.00000001 )
  real, save :: f
  integer me

  include 'shmem.fh'

  call start_pes(0)
  me = my_pe()

  f = 3.1415927

  call shmem_barrier_all()

  if (me == 0) then
     call shmem_real_put(f, e, 1, 1)
  end if

 call shmem_barrier_all()
  
  if (me == 1) then
     if (abs(f - e) < epsilon) then
        print *, 'OK'
     else
        print *, 'FAIL'
     end if
  end if

end program fip
