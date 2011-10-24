! (c) 2011 University of Houston System.  All rights reserved. 
!
!
!
! Test whether various types of variables are accessible
! Test if all PEs are accessible

program test_shmem_accessible
  implicit none
  include 'mpp/shmem.fh'

  integer*8          :: global_target(1)
  integer*8, save    :: static_target(1)

  integer*8          :: local_target(1)

  ! Cray-Pointer for dynamically allocatd data 
  integer*8          :: ptr
  integer*8          :: shm_target(1)    
  pointer            (ptr, shm_target)
  ! --
  
  integer            :: me, npes, i
  logical            :: pe_acc_success
  
  integer            :: errcode, abort, length
  
  common /globalvars/   global_target

  ! SHMEM function definitions
  integer                   :: my_pe, num_pes
  ! --

  static_target  = 1
  pe_acc_success = .false.
  
  call start_pes(0)
  me   = my_pe()
  npes = num_pes()
   
  length = 1

  call shpalloc(ptr, length, errcode, abort)  

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
    
    do i = 0, npes - 1, 1
      pe_acc_success = shmem_pe_accessible(i)
      if(.not.pe_acc_success) then
        write (*,*) "Test shmem_pe_accessible: Failed"
        stop
      end if
    end do
    write (*,*) "Test shmem_pe_accessible: Passed"
    
  end if

  call shpdeallc(ptr, errcode, abort)
  
contains
  
  logical function check_it_arr (addr)
    implicit none
    integer*8 :: addr(*)
    logical   :: is_acc, shmem_addr_accessible
  
    is_acc = shmem_addr_accessible(addr, 1)
  
    check_it_arr = is_acc
  end function check_it_arr

  logical function check_it (addr)
    implicit none
    integer*8 :: addr(*)
    logical   :: is_acc, shmem_addr_accessible
  
    is_acc = shmem_addr_accessible(addr, 1)
  
    check_it = is_acc
  end function check_it
end program test_shmem_accessible
