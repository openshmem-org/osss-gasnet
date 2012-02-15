
c---------------------------------------------------------------------
c---------------------------------------------------------------------

       subroutine copy_faces

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c this function copies the face values of a variable defined on a set 
c of cells to the overlap locations of the adjacent sets of cells. 
c Because a set of cells interfaces in each direction with exactly one 
c other set, we only need to fill six different buffers. We could try to 
c overlap communication with computation, by computing
c some internal values while communicating boundary values, but this
c adds so much overhead that it's not clearly useful. 
c---------------------------------------------------------------------

       include 'header.h'

       integer i, j, k, c, m, p0, p1, error,
     >         p2, p3, p4, p5, b_size(0:5), ss(0:5), 
     >         sr(0:5), sr_n(0:5)

c---------------------------------------------------------------------
c      exit immediately if there are no faces to be copied           
c---------------------------------------------------------------------
       if (no_nodes .eq. 1) then
          call compute_rhs
          return
       endif


       ss(0) = start_send_east
       ss(1) = start_send_west
       ss(2) = start_send_north
       ss(3) = start_send_south
       ss(4) = start_send_top
       ss(5) = start_send_bottom

       sr(0) = start_recv_east
       sr(1) = start_recv_west
       sr(2) = start_recv_north
       sr(3) = start_recv_south
       sr(4) = start_recv_top
       sr(5) = start_recv_bottom

       sr_n(0) =   successor(1)
       sr_n(1) = predecessor(1)
       sr_n(2) =   successor(2)
       sr_n(3) = predecessor(2)
       sr_n(4) =   successor(3)
       sr_n(5) = predecessor(3)

       b_size(0) = east_size
       b_size(1) = west_size
       b_size(2) = north_size
       b_size(3) = south_size
       b_size(4) = top_size
       b_size(5) = bottom_size

c---------------------------------------------------------------------
c because the difference stencil for the diagonalized scheme is 
c orthogonal, we do not have to perform the staged copying of faces, 
c but can send all face information simultaneously to the neighboring 
c cells in all directions          
c---------------------------------------------------------------------
       if (timeron) call timer_start(t_bpack)
       p0 = 0
       p1 = 0
       p2 = 0
       p3 = 0
       p4 = 0
       p5 = 0

       do  c = 1, ncells
          do   m = 1, 5

c---------------------------------------------------------------------
c            fill the buffer to be sent to eastern neighbors (i-dir)
c---------------------------------------------------------------------
             if (cell_coord(1,c) .ne. ncells) then
                do   k = 0, cell_size(3,c)-1
                   do   j = 0, cell_size(2,c)-1
                      do   i = cell_size(1,c)-2, cell_size(1,c)-1
                         out_buffer(ss(0)+p0) = u(i,j,k,m,c)
                         p0 = p0 + 1
                      end do
                   end do
                end do
             endif

c---------------------------------------------------------------------
c            fill the buffer to be sent to western neighbors 
c---------------------------------------------------------------------
             if (cell_coord(1,c) .ne. 1) then
                do   k = 0, cell_size(3,c)-1
                   do   j = 0, cell_size(2,c)-1
                      do   i = 0, 1
                         out_buffer(ss(1)+p1) = u(i,j,k,m,c)
                         p1 = p1 + 1
                      end do
                   end do
                end do


             endif

c---------------------------------------------------------------------
c            fill the buffer to be sent to northern neighbors (j_dir)
c---------------------------------------------------------------------
             if (cell_coord(2,c) .ne. ncells) then
                do   k = 0, cell_size(3,c)-1
                   do   j = cell_size(2,c)-2, cell_size(2,c)-1
                      do   i = 0, cell_size(1,c)-1
                         out_buffer(ss(2)+p2) = u(i,j,k,m,c)
                         p2 = p2 + 1
                      end do
                   end do
                end do
             endif

c---------------------------------------------------------------------
c            fill the buffer to be sent to southern neighbors 
c---------------------------------------------------------------------
             if (cell_coord(2,c).ne. 1) then
                do   k = 0, cell_size(3,c)-1
                   do   j = 0, 1
                      do   i = 0, cell_size(1,c)-1   
                         out_buffer(ss(3)+p3) = u(i,j,k,m,c)
                         p3 = p3 + 1
                      end do
                   end do
                end do
             endif

c---------------------------------------------------------------------
c            fill the buffer to be sent to top neighbors (k-dir)
c---------------------------------------------------------------------
             if (cell_coord(3,c) .ne. ncells) then
                do   k = cell_size(3,c)-2, cell_size(3,c)-1
                   do   j = 0, cell_size(2,c)-1
                      do   i = 0, cell_size(1,c)-1
                         out_buffer(ss(4)+p4) = u(i,j,k,m,c)
                         p4 = p4 + 1
                      end do
                   end do
                end do
             endif

c---------------------------------------------------------------------
c            fill the buffer to be sent to bottom neighbors
c---------------------------------------------------------------------
             if (cell_coord(3,c).ne. 1) then
                 do    k=0, 1
                    do   j = 0, cell_size(2,c)-1
                       do   i = 0, cell_size(1,c)-1
                          out_buffer(ss(5)+p5) = u(i,j,k,m,c)
                          p5 = p5 + 1
                       end do
                    end do
                 end do
              endif

c---------------------------------------------------------------------
c          m loop
c---------------------------------------------------------------------
           end do

c---------------------------------------------------------------------
c       cell loop
c---------------------------------------------------------------------
        end do
       if (timeron) call timer_stop(t_bpack)

c .... synchronize all updates and get remote data
       if (timeron) call timer_start(t_exch)
c       call mpi_win_fence(MPI_MODE_NOPUT+MPI_MODE_NOPRECEDE, win, error)
       call shmem_barrier_all()

       do c = 0, 5
          disp = sr(c)
c          call mpi_get(in_buffer(sr(c)), b_size(c), dp_type,
c     >       sr_n(c), disp, b_size(c), dp_type, win, error)
         call shmem_double_get(in_buffer(sr(c)),
     >   out_buffer(disp), b_size(c), sr_n(c))
       end do
c       call mpi_win_fence(MPI_MODE_NOSUCCEED, win, error)
      call shmem_barrier_all()

       if (timeron) call timer_stop(t_exch)

c---------------------------------------------------------------------
c unpack the data that has just been received;             
c---------------------------------------------------------------------
       if (timeron) call timer_start(t_bpack)
       p0 = 0
       p1 = 0
       p2 = 0
       p3 = 0
       p4 = 0
       p5 = 0

       do   c = 1, ncells
          do    m = 1, 5

             if (cell_coord(1,c) .ne. 1) then
                do   k = 0, cell_size(3,c)-1
                   do   j = 0, cell_size(2,c)-1
                      do   i = -2, -1
                         u(i,j,k,m,c) = in_buffer(sr(1)+p0)
                         p0 = p0 + 1
                      end do
                   end do
                end do
             endif

             if (cell_coord(1,c) .ne. ncells) then
                do  k = 0, cell_size(3,c)-1
                   do  j = 0, cell_size(2,c)-1
                      do  i = cell_size(1,c), cell_size(1,c)+1
                         u(i,j,k,m,c) = in_buffer(sr(0)+p1)
                         p1 = p1 + 1
                      end do
                   end do
                end do
             end if
 
             if (cell_coord(2,c) .ne. 1) then
                do  k = 0, cell_size(3,c)-1
                   do   j = -2, -1
                      do  i = 0, cell_size(1,c)-1
                         u(i,j,k,m,c) = in_buffer(sr(3)+p2)
                         p2 = p2 + 1
                      end do
                   end do
                end do

             endif
 
             if (cell_coord(2,c) .ne. ncells) then
                do  k = 0, cell_size(3,c)-1
                   do   j = cell_size(2,c), cell_size(2,c)+1
                      do  i = 0, cell_size(1,c)-1
                         u(i,j,k,m,c) = in_buffer(sr(2)+p3)
                         p3 = p3 + 1
                      end do
                   end do
                end do
             endif

             if (cell_coord(3,c) .ne. 1) then
                do  k = -2, -1
                   do  j = 0, cell_size(2,c)-1
                      do  i = 0, cell_size(1,c)-1
                         u(i,j,k,m,c) = in_buffer(sr(5)+p4)
                         p4 = p4 + 1
                      end do
                   end do
                end do
             endif

             if (cell_coord(3,c) .ne. ncells) then
                do  k = cell_size(3,c), cell_size(3,c)+1
                   do  j = 0, cell_size(2,c)-1
                      do  i = 0, cell_size(1,c)-1
                         u(i,j,k,m,c) = in_buffer(sr(4)+p5)
                         p5 = p5 + 1
                      end do
                   end do
                end do
             endif

c---------------------------------------------------------------------
c         m loop            
c---------------------------------------------------------------------
          end do

c---------------------------------------------------------------------
c      cells loop
c---------------------------------------------------------------------
       end do
       if (timeron) call timer_stop(t_bpack)

c---------------------------------------------------------------------
c now that we have all the data, compute the rhs
c---------------------------------------------------------------------
       call compute_rhs

       return
       end
