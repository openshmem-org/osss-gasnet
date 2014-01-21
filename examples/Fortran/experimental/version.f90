! (c) 2011 University of Houston System.  All rights reserved.


!
! simply prints out the version of OpenSHMEM from PE 0 only
!

program version

  include 'mpp/shmem.fh'

  integer :: shmem_my_pe, shmem_n_pes

  integer :: me
  integer :: maj, min

  call start_pes(0)
  me = shmem_my_pe()

  if (me == 0) then
     call shmem_version(maj, min)
     write (*, *) 'PE ', me, ' says hello from OpenSHMEM version'
     write (*, *) 'major ', maj, 'minor ',  min
  end if

end program version
