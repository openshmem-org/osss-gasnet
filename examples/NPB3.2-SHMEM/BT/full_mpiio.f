
c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine setup_btio

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      include 'header.h'
      include 'mpinpb.h'

      integer ierr
      integer mstatus(MPI_STATUS_SIZE)
      integer sizes(4), starts(4), subsizes(4)
      integer cell_btype(maxcells), cell_ftype(maxcells)
      integer cell_blength(maxcells)
      integer info
      character*20 cb_nodes, cb_size
      integer c
      integer cell_disp(maxcells)

       call mpi_bcast(collbuf_nodes, 1, MPI_INTEGER,
     >                root, comm_setup, ierr)

       call mpi_bcast(collbuf_size, 1, MPI_INTEGER,
     >                root, comm_setup, ierr)

       if (collbuf_nodes .eq. 0) then
          info = MPI_INFO_NULL
       else
          write (cb_nodes,*) collbuf_nodes
          write (cb_size,*) collbuf_size
          call MPI_Info_create(info, ierr)
          call MPI_Info_set(info, 'cb_nodes', cb_nodes, ierr)
          call MPI_Info_set(info, 'cb_buffer_size', cb_size, ierr)
          call MPI_Info_set(info, 'collective_buffering', 'true', ierr)
       endif

       call MPI_Type_contiguous(5, MPI_DOUBLE_PRECISION,
     $                          element, ierr)
       call MPI_Type_commit(element, ierr)
       call MPI_Type_extent(element, eltext, ierr)

       do  c = 1, ncells
c
c Outer array dimensions ar same for every cell
c
	   sizes(1) = IMAX+4
           sizes(2) = JMAX+4
           sizes(3) = KMAX+4
c
c 4th dimension is cell number, total of maxcells cells
c
	   sizes(4) = maxcells
c
c Internal dimensions of cells can differ slightly between cells
c
           subsizes(1) = cell_size(1, c)
           subsizes(2) = cell_size(2, c)
           subsizes(3) = cell_size(3, c)
c
c Cell is 4th dimension, 1 cell per cell type to handle varying 
c cell sub-array sizes
c
	   subsizes(4) = 1

c
c type constructors use 0-based start addresses
c
           starts(1) = 2 
           starts(2) = 2
           starts(3) = 2
	   starts(4) = c-1

c 
c Create buftype for a cell
c
           call MPI_Type_create_subarray(4, sizes, subsizes, 
     $          starts, MPI_ORDER_FORTRAN, element, 
     $          cell_btype(c), ierr)
c
c block length and displacement for joining cells - 
c 1 cell buftype per block, cell buftypes have own displacment
c generated from cell number (4th array dimension)
c 	
	   cell_blength(c) = 1
           cell_disp(c) = 0

       enddo
c
c Create combined buftype for all cells
c
       call MPI_Type_struct(ncells, cell_blength, cell_disp,
     $            cell_btype, combined_btype, ierr)
       call MPI_Type_commit(combined_btype, ierr)

       do  c = 1, ncells
c
c Entire array size
c
           sizes(1) = PROBLEM_SIZE
           sizes(2) = PROBLEM_SIZE
           sizes(3) = PROBLEM_SIZE

c
c Size of c'th cell
c
           subsizes(1) = cell_size(1, c)
           subsizes(2) = cell_size(2, c)
           subsizes(3) = cell_size(3, c)

c
c Starting point in full array of c'th cell
c
           starts(1) = cell_low(1,c)
           starts(2) = cell_low(2,c)
           starts(3) = cell_low(3,c)

           call MPI_Type_create_subarray(3, sizes, subsizes,
     $          starts, MPI_ORDER_FORTRAN,
     $          element, cell_ftype(c), ierr)
           cell_blength(c) = 1
           cell_disp(c) = 0
       enddo

       call MPI_Type_struct(ncells, cell_blength, cell_disp,
     $            cell_ftype, combined_ftype, ierr)
       call MPI_Type_commit(combined_ftype, ierr)

       iseek=0
       if (node .eq. root) then
          call MPI_File_delete(filenm, MPI_INFO_NULL, ierr)
       endif


      call MPI_Barrier(comm_solve, ierr)

       call MPI_File_open(comm_solve,
     $          filenm,
     $          MPI_MODE_WRONLY+MPI_MODE_CREATE,
     $          MPI_INFO_NULL, fp, ierr)

       if (ierr .ne. MPI_SUCCESS) then
		print *, 'Error opening file'
        	stop
       endif

	call MPI_File_set_view(fp, iseek, element, 
     $		combined_ftype, 'native', info, ierr)

       if (ierr .ne. MPI_SUCCESS) then
		print *, 'Error setting file view'
        	stop
       endif


      return
      end

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine output_timestep

c---------------------------------------------------------------------
c---------------------------------------------------------------------
      include 'header.h'
      include 'mpinpb.h'

      integer mstatus(MPI_STATUS_SIZE)
      integer ierr

      call MPI_File_write_at_all(fp, iseek, u,
     $                           1, combined_btype, mstatus, ierr)
      if (ierr .ne. MPI_SUCCESS) then
          print *, 'Error writing to file'
          stop
      endif

      call MPI_Type_size(combined_btype, iosize, ierr)
      iseek = iseek + iosize/eltext

      return
      end

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine btio_cleanup

c---------------------------------------------------------------------
c---------------------------------------------------------------------
      include 'header.h'
      include 'mpinpb.h'

      integer ierr

      call MPI_File_close(fp, ierr)

      return
      end

c---------------------------------------------------------------------
c---------------------------------------------------------------------


      subroutine accumulate_norms(xce_acc)

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      include 'header.h'
      include 'mpinpb.h'

      integer count, jio, kio, cio, m
      integer ierr
      integer mstatus(MPI_STATUS_SIZE)

      double precision xce_acc(5), xce_single(5)
      integer ii

      call MPI_File_open(comm_solve,
     $          filenm,
     $          MPI_MODE_RDONLY,
     $          MPI_INFO_NULL,
     $          fp,
     $          ierr)

      iseek = 0
      call MPI_File_set_view(fp, iseek, element, combined_ftype,
     $          'native', MPI_INFO_NULL, ierr)

c     clear the last time step

      call clear_timestep

c     read back the time steps and accumulate norms

      do m = 1, 5
         xce_acc(m) = 0.d0
      end do

      do ii=0, idump-1

        call MPI_File_read_at_all(fp, iseek, u,
     $                           1, combined_btype, mstatus, ierr)
        if (ierr .ne. MPI_SUCCESS) then
           print *, 'Error reading back file'
           call MPI_File_close(fp, ierr)
           stop
        endif

        if (node .eq. root) print *, 'Reading data set ', ii+1

        call error_norm(xce_single)
        do m = 1, 5
           xce_acc(m) = xce_acc(m) + xce_single(m)
        end do

        call MPI_Type_size(combined_btype, iosize, ierr)
        iseek = iseek + iosize/eltext

      enddo

      do m = 1, 5
         xce_acc(m) = xce_acc(m) / dble(idump)
      end do

      call MPI_File_close(fp, ierr)

      return
      end

