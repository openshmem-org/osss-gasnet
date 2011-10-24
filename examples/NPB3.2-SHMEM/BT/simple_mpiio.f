
c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine setup_btio

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      include 'header.h'
      include 'mpinpb.h'

      integer ierr

      iseek=0

      if (node .eq. root) then
          call MPI_File_delete(filenm, MPI_INFO_NULL, ierr)
      endif

      call MPI_Barrier(comm_solve, ierr)

      call MPI_File_open(comm_solve,
     $          filenm,
     $          MPI_MODE_WRONLY + MPI_MODE_CREATE,
     $          MPI_INFO_NULL,
     $          fp,
     $          ierr)

      call MPI_File_set_view(fp,
     $          iseek, MPI_DOUBLE_PRECISION, MPI_DOUBLE_PRECISION,
     $          'native', MPI_INFO_NULL, ierr)

      if (ierr .ne. MPI_SUCCESS) then
          print *, 'Error opening file'
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

      integer count, jio, kio, cio, aio
      integer ierr
      integer mstatus(MPI_STATUS_SIZE)

      do cio=1,ncells
          do kio=0, cell_size(3,cio)-1
              do jio=0, cell_size(2,cio)-1
                  iseek=5*(cell_low(1,cio) +
     $                   PROBLEM_SIZE*((cell_low(2,cio)+jio) +
     $                   PROBLEM_SIZE*((cell_low(3,cio)+kio) +
     $                   PROBLEM_SIZE*idump)))

                  count=5*cell_size(1,cio)

                  call MPI_File_write_at(fp, iseek,
     $                  u(1,0,jio,kio,cio),
     $                  count, MPI_DOUBLE_PRECISION,
     $                  mstatus, ierr)

                  if (ierr .ne. MPI_SUCCESS) then
                      print *, 'Error writing to file'
                      stop
                  endif
              enddo
          enddo
      enddo

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

      integer count, jio, kio, cio, aio, m
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
      call MPI_File_set_view(fp,
     $          iseek, MPI_DOUBLE_PRECISION, MPI_DOUBLE_PRECISION,
     $          'native', MPI_INFO_NULL, ierr)

c     clear the last time step

      call clear_timestep

c     read back the time steps and accumulate norms

      do m = 1, 5
         xce_acc(m) = 0.d0
      end do

      do ii=0, idump-1
        do cio=1,ncells
          do kio=0, cell_size(3,cio)-1
              do jio=0, cell_size(2,cio)-1
                  iseek=5*(cell_low(1,cio) +
     $                   PROBLEM_SIZE*((cell_low(2,cio)+jio) +
     $                   PROBLEM_SIZE*((cell_low(3,cio)+kio) +
     $                   PROBLEM_SIZE*ii)))

                  count=5*cell_size(1,cio)

                  call MPI_File_read_at(fp, iseek,
     $                  u(1,0,jio,kio,cio),
     $                  count, MPI_DOUBLE_PRECISION,
     $                  mstatus, ierr)

                  if (ierr .ne. MPI_SUCCESS) then
                      print *, 'Error reading back file'
                      call MPI_File_close(fp, ierr)
                      stop
                  endif
              enddo
          enddo
        enddo

        if (node .eq. root) print *, 'Reading data set ', ii+1

        call error_norm(xce_single)
        do m = 1, 5
           xce_acc(m) = xce_acc(m) + xce_single(m)
        end do
      enddo

      do m = 1, 5
         xce_acc(m) = xce_acc(m) / dble(idump)
      end do

      call MPI_File_close(fp, ierr)

      return
      end

