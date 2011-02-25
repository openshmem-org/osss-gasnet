program cswap

  include 'mpp/shmem.fh'

  integer, save :: race_winner
  integer oldval
  integer me

  call start_pes(0)
  me = my_pe()

  race_winner = -1
  call shmem_barrier_all()

  oldval = shmem_int4_cswap(race_winner, -1, me, 0)

  if (oldval == -1) then
     print *, 'pe ', me, ' was first'
  end if

end program cswap
