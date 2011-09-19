program test_shmem_shpalloc
  implicit none
  include 'mpp/shmem.fh'

  integer, parameter :: N = 3
  integer, pointer   :: dest
  integer            :: src(N)

  integer            :: errcode, abort, me, npes, i

  me  = my_pe()
  npes = num_pes()

  call shpalloc(dest, N, errcode, abort)
  
  write (*,*) 'allocated..'

  do i = 1, N, 1
    src(i) = me
  end do

  call shmem_barrier_all()

  call shmem_integer_put(dest, src, 3, 1)

  call shmem_barrier_all()

  if(me .eq. 1) then
    write (*,*) dest
  end if

  call shmem_barrier_all()

  call shpdeallc(dest)

end program
