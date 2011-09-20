program test_shmem_shpalloc
  implicit none
  include 'mpp/shmem.fh'

  integer, parameter :: N = 3

  integer*8          :: ptr
  integer            :: dst(*)
  pointer            (ptr, dst)
  integer            :: dst
  integer            :: src(N)
  integer            :: success ! this should probably be renamed to failed :)

  integer            :: errcode, abort, me, npes, i

! Function definitons
  integer    :: my_pe, num_pes

  me  = my_pe()
  npes = num_pes()

  call shpalloc(ptr, N, errcode, abort)
  do i = 1, N, 1
    dst(i) = -9
    src(i) = me
  end do


  call shmem_barrier_all()

  if(me .eq. 1) then
    !call shmem_integer_put(dst, src, N, 0)
    call shmem_integer_put(dst, src, N, 0)
  end if

  call shmem_barrier_all()

  success = 0
  if(me .eq. 0) then
    do i = 1, N, 1
      if(dst(i) .ne. 1) then
        success = 1
      end if
    end do
  end if

  if(success .eq. 0) then
    write (*,*) "Test shpalloc(): Passed"
  else
    write (*,*) "Test shpalloc(): Failed"
  end if

  call shmem_barrier_all()

  call shpdeallc(ptr)

end program
