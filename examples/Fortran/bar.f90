!
! man page (SGI) says barrier is synonym for shmem_barrier_all, but isn't
! true (confirmed by SGI)
!

program testbarrier

  call start_pes(0)

  call shmem_barrier_all

end program testbarrier
