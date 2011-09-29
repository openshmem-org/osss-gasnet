! Attempts to allocate an array in the symmetric heap.
! The test passes if we can correctly PUT data into that object on a remote PE.
program test_shmem_shpalloc
  implicit none
  include 'mpp/shmem.fh'

  integer, parameter :: N = 3

  integer*8          :: ptr
  integer            :: dst(1)    
  pointer            (ptr, dst)
  integer            :: src(N)
  integer            :: success ! this should probably be renamed to failed :)

  integer            :: errcode, abort, me, npes, i

! Function definitons
  integer    :: my_pe, num_pes

  call start_pes(0)

  me  = my_pe()
  npes = num_pes()

  call shpalloc(ptr, N, errcode, abort)

  do i = 1, N, 1
    dst(i) = -9
    src(i) = me
  end do

  call shmem_barrier_all()

  if(me .eq. 1) then
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

	if(me .eq. 0) then
	  if(success .eq. 0) then
	    write (*,*) "Test shpalloc(): Passed"
	  else
	    write (*,*) "Test shpalloc(): Failed"
	  end if
	end if
	
  call shmem_barrier_all()

  call shpdeallc(ptr, errcode, abort)
  
  ! TODO: test if shpdeallc really deallocates memory

end program
