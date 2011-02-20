!
! test initializing library multiple times
!

program multiinit

  include 'shmem.fh'

  call start_pes(0)
  call start_pes(0)

end program multiinit
