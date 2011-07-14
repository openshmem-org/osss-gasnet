! (c) 2011 University of Houston System.  All rights reserved.


!
! the ubiquitous hello world program
!

program whoami

  include 'shmem.fh'

  integer npes, me
  character*32 h

  call start_pes(0)

  npes = num_pes()
  me = my_pe()
  call hostnm(h)

  print *, h, 'I am ', me, ' of ', npes

end program whoami
