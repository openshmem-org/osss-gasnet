!
! simply prints out the version of OpenSHMEM from PE 0 only
!

program version

  integer me

  include 'mpp/shmem.fh'

  call start_pes(0)
  me = my_pe()

  if (me == 0) then
     print *, 'PE ', me, ' says hello from'
     print *, '    SHMEM library version ', shmem_version()
  end if

end program version
