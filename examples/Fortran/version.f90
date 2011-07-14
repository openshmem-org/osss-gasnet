! (c) 2011 University of Houston System.  All rights reserved.


!
! simply prints out the version of OpenSHMEM from PE 0 only
!

program version

  integer me
  integer maj, min

  include 'mpp/shmem.fh'

  call start_pes(0)
  me = my_pe()

  if (me == 0) then
     call shmem_version(maj, min)
     write (*, *) 'PE ', me, ' says hello from OpenSHMEM version'
     write (*, *) 'major ', maj, 'minor ',  min
  end if

end program version
