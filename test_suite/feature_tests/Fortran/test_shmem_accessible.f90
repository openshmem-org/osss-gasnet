! (c) 2011 University of Houston System.  All rights reserved. 
!
!
!
! Test whether various types of variables are accessible
! Test if all PEs are accessible

program test_shmem_accessible
  implicit none
  include 'mpp/shmem.fh'

  integer*8          :: global_target
  integer*8, save    :: static_target

  integer*8          :: local_target
  integer*8, pointer :: shm_target
  
  integer            :: me, npes, i
  logical            :: pe_acc_success
  
  integer            :: errcode, abort, length
  
  common /globalvars/   global_target
  
  static_target  = 1
  pe_acc_success = .false.
  
  call start_pes(0)
  me   = my_pe()
  npes = num_pes()
   
  length = 1
! Fortran dynamic allocation is not working correctly yet.  
! call shpalloc(shm_target, length, errcode, abort)  

  call shmem_barrier_all()
  
  if (me .eq. 0) then
    if (.not.check_it(global_target)) then ! long global: yes 
      write (*,*) "Test Global Address Accessable: Failed"  
    else
      write (*,*) "Test Global Address Accessable: Passed"
    end if
  
    if (.not.check_it(static_target)) then ! static int global: yes
      write (*,*) "Test Static Global Address Accessable: Failed"
    else 
      write (*,*) "Test Static Global Address Accessable: Passed"
    end if
  
    if (check_it(local_target)) then ! main program stack: no
      write (*,*) "Test Stack Address Accessable: Failed"
    else
      write (*,*) "Test Stack Address Accessable: Passed"
    end if
    if (.not.check_it(shm_target)) then ! shmalloc: yes */
      write (*,*) "Test Shmalloc-ed Address Accessable: Failed"
    else
      write (*,*) "Test Shmalloc-ed Address Accessable: Passed" 
    end if
    
    do i = 1, npes, 1
      pe_acc_success = shmem_pe_accessible(i)
      if(.not.pe_acc_success) then
        write (*,*) "Test shmem_pe_accessible: Failed"
        stop
      end if
    end do
    write (*,*) "Test shmem_pe_accessible: Passed"
    
  end if

! Fortran dynamic allocation is not working correctly yet.
! call shpdeallc(shm_target, errcode, abort)
  
contains
  
  logical function check_it (addr)
    implicit none
    integer*8 :: addr
    logical   :: is_acc, shmem_addr_accessible
  
    is_acc = shmem_addr_accessible(addr, 1)
  
    check_it = is_acc
  end function check_it
end program test_shmem_accessible
