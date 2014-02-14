!
! This example adapted from Cray's man pages:
!
!   http://docs.cray.com/cgi-bin/craydoc.cgi?mode=View;id=sw_releases-dj1kd1jh-1382363518;idx=man_search;this_sort=title;q=;type=man;title=Message%20Passing%20Toolkit%20%28MPT%29%206.2%20Man%20Pages
!
! Original copyright indicated at above URL.
!
! Changes here are to make it OpenSHMEM compliant (e.g. start_pes
! instead of shmem_init, can't use my_pe as variable because it
! conflicts with API).
!
! Requires "cray pointer" compatibility (e.g. -fcray-pointer with GCC).
!

program shpalloc_example
  implicit none
  include "mpp/shmem.fh"
  integer :: num_pes    ! not declared in SGI header

  integer :: n_pes
  integer :: imax, jmax
  call start_pes(0)
  n_pes = num_pes()
  imax  = n_pes
  jmax  = n_pes
  call sub(imax,jmax)
end program shpalloc_example

subroutine sub(imax,jmax)
  implicit none
  include "mpp/shmem.fh"
  integer :: my_pe, num_pes    ! not declared in SGI header

  integer imax, jmax
  integer :: mype, n_pes
  integer :: pe, err, idefault, ibytes
  pointer (pA,A)
  integer(kind=8) A(imax,jmax)
  mype = my_pe()
  n_pes = num_pes()
  pe = n_pes - mype - 1
  ibytes=kind(idefault)
  !          want to create matrix of size A(imax,jmax)
  call shpalloc(pA,imax*jmax*8/ibytes,err,0)
  A(imax,jmax) = mype
  call shmem_barrier_all()
  call shmem_put64(A(1,1), A(imax,jmax), 1, pe)
  call shmem_barrier_all()
  print *, mype, A(1,1)
  call shmem_barrier_all()
  call shpdeallc(pA,err,0)
  return
end subroutine sub
