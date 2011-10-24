
c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine x_solve

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c     
c     Performs line solves in X direction by first factoring
c     the block-tridiagonal matrix into an upper triangular matrix, 
c     and then performing back substitution to solve for the unknow
c     vectors of each line.  
c     
c     Make sure we treat elements zero to cell_size in the direction
c     of the sweep.
c     
c---------------------------------------------------------------------

      include 'header.h'
      include 'mpinpb.h'
      integer  c, istart, stage,
     >     first, last, recv_id, error, r_status(MPI_STATUS_SIZE),
     >     isize,jsize,ksize,send_id

      istart = 0

c---------------------------------------------------------------------
c     in our terminology stage is the number of the cell in the x-direction
c     i.e. stage = 1 means the start of the line stage=ncells means end
c---------------------------------------------------------------------
      do stage = 1,ncells
         c = slice(1,stage)
         isize = cell_size(1,c) - 1
         jsize = cell_size(2,c) - 1
         ksize = cell_size(3,c) - 1
         
c---------------------------------------------------------------------
c     set last-cell flag
c---------------------------------------------------------------------
         if (stage .eq. ncells) then
            last = 1
         else
            last = 0
         endif

         if (stage .eq. 1) then
c---------------------------------------------------------------------
c     This is the first cell, so solve without receiving data
c---------------------------------------------------------------------
            first = 1
c            call lhsx(c)
            call x_solve_cell(first,last,c)
         else
c---------------------------------------------------------------------
c     Not the first cell of this line, so receive info from
c     processor working on preceeding cell
c---------------------------------------------------------------------
            first = 0
            call x_receive_solve_info(recv_id,c)
c---------------------------------------------------------------------
c     overlap computations and communications
c---------------------------------------------------------------------
c            call lhsx(c)
c---------------------------------------------------------------------
c     wait for completion
c---------------------------------------------------------------------
            call mpi_wait(send_id,r_status,error)
            call mpi_wait(recv_id,r_status,error)
c---------------------------------------------------------------------
c     install C'(istart) and rhs'(istart) to be used in this cell
c---------------------------------------------------------------------
            call x_unpack_solve_info(c)
            call x_solve_cell(first,last,c)
         endif

         if (last .eq. 0) call x_send_solve_info(send_id,c)
      enddo

c---------------------------------------------------------------------
c     now perform backsubstitution in reverse direction
c---------------------------------------------------------------------
      do stage = ncells, 1, -1
         c = slice(1,stage)
         first = 0
         last = 0
         if (stage .eq. 1) first = 1
         if (stage .eq. ncells) then
            last = 1
c---------------------------------------------------------------------
c     last cell, so perform back substitute without waiting
c---------------------------------------------------------------------
            call x_backsubstitute(first, last,c)
         else
            call x_receive_backsub_info(recv_id,c)
            call mpi_wait(send_id,r_status,error)
            call mpi_wait(recv_id,r_status,error)
            call x_unpack_backsub_info(c)
            call x_backsubstitute(first,last,c)
         endif
         if (first .eq. 0) call x_send_backsub_info(send_id,c)
      enddo
10000 format (6(2x,e8.2))

      return
      end
      
      
c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine x_unpack_solve_info(c)

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c     unpack C'(-1) and rhs'(-1) for
c     all j and k
c---------------------------------------------------------------------

      include 'header.h'
      integer j,k,m,n,ptr,c,istart 

      istart = 0
      ptr = 0
      do k=0,KMAX-1
      	 do j=0,JMAX-1
            do m=1,BLOCK_SIZE
               do n=1,BLOCK_SIZE
                  lhsc(m,n,istart-1,j,k,c) = out_buffer(ptr+n)
               enddo
               ptr = ptr+BLOCK_SIZE
            enddo
            do n=1,BLOCK_SIZE
               rhs(n,istart-1,j,k,c) = out_buffer(ptr+n)
            enddo
            ptr = ptr+BLOCK_SIZE
         enddo
      enddo

      return
      end

c---------------------------------------------------------------------
c---------------------------------------------------------------------
      
      subroutine x_send_solve_info(send_id,c)

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c     pack up and send C'(iend) and rhs'(iend) for
c     all j and k
c---------------------------------------------------------------------

      include 'header.h'
      include 'mpinpb.h'

      integer j,k,m,n,isize,ptr,c,jp,kp
      integer error,send_id,buffer_size 

      isize = cell_size(1,c)-1
      jp = cell_coord(2,c) - 1
      kp = cell_coord(3,c) - 1
      buffer_size=MAX_CELL_DIM*MAX_CELL_DIM*
     >     (BLOCK_SIZE*BLOCK_SIZE + BLOCK_SIZE)

c---------------------------------------------------------------------
c     pack up buffer
c---------------------------------------------------------------------
      ptr = 0
      do k=0,KMAX-1
      	 do j=0,JMAX-1
            do m=1,BLOCK_SIZE
               do n=1,BLOCK_SIZE
                  in_buffer(ptr+n) = lhsc(m,n,isize,j,k,c)
               enddo
               ptr = ptr+BLOCK_SIZE
            enddo
            do n=1,BLOCK_SIZE
               in_buffer(ptr+n) = rhs(n,isize,j,k,c)
            enddo
            ptr = ptr+BLOCK_SIZE
         enddo
      enddo

c---------------------------------------------------------------------
c     send buffer 
c---------------------------------------------------------------------
      call mpi_isend(in_buffer, buffer_size,
     >     dp_type, successor(1),
     >     WEST+jp+kp*NCELLS, comm_solve,
     >     send_id,error)

      return
      end

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine x_send_backsub_info(send_id,c)

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c     pack up and send U(istart) for all j and k
c---------------------------------------------------------------------

      include 'header.h'
      include 'mpinpb.h'

      integer j,k,n,ptr,c,istart,jp,kp
      integer error,send_id,buffer_size

c---------------------------------------------------------------------
c     Send element 0 to previous processor
c---------------------------------------------------------------------
      istart = 0
      jp = cell_coord(2,c)-1
      kp = cell_coord(3,c)-1
      buffer_size=MAX_CELL_DIM*MAX_CELL_DIM*BLOCK_SIZE
      ptr = 0
      do k=0,KMAX-1
      	 do j=0,JMAX-1
            do n=1,BLOCK_SIZE
               in_buffer(ptr+n) = rhs(n,istart,j,k,c)
            enddo
            ptr = ptr+BLOCK_SIZE
         enddo
      enddo
      call mpi_isend(in_buffer, buffer_size,
     >     dp_type, predecessor(1), 
     >     EAST+jp+kp*NCELLS, comm_solve, 
     >     send_id,error)

      return
      end

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine x_unpack_backsub_info(c)

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c     unpack U(isize) for all j and k
c---------------------------------------------------------------------

      include 'header.h'
      integer j,k,n,ptr,c

      ptr = 0
      do k=0,KMAX-1
      	 do j=0,JMAX-1
            do n=1,BLOCK_SIZE
               backsub_info(n,j,k,c) = out_buffer(ptr+n)
            enddo
            ptr = ptr+BLOCK_SIZE
         enddo
      enddo

      return
      end

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine x_receive_backsub_info(recv_id,c)

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c     post mpi receives
c---------------------------------------------------------------------

      include 'header.h'
      include 'mpinpb.h'

      integer error,recv_id,jp,kp,c,buffer_size
      jp = cell_coord(2,c) - 1
      kp = cell_coord(3,c) - 1
      buffer_size=MAX_CELL_DIM*MAX_CELL_DIM*BLOCK_SIZE
      call mpi_irecv(out_buffer, buffer_size,
     >     dp_type, successor(1), 
     >     EAST+jp+kp*NCELLS, comm_solve, 
     >     recv_id, error)

      return
      end

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine x_receive_solve_info(recv_id,c)

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c     post mpi receives 
c---------------------------------------------------------------------

      include 'header.h'
      include 'mpinpb.h'

      integer jp,kp,recv_id,error,c,buffer_size
      jp = cell_coord(2,c) - 1
      kp = cell_coord(3,c) - 1
      buffer_size=MAX_CELL_DIM*MAX_CELL_DIM*
     >     (BLOCK_SIZE*BLOCK_SIZE + BLOCK_SIZE)
      call mpi_irecv(out_buffer, buffer_size,
     >     dp_type, predecessor(1), 
     >     WEST+jp+kp*NCELLS,  comm_solve, 
     >     recv_id, error)

      return
      end

c---------------------------------------------------------------------
c---------------------------------------------------------------------
      
      subroutine x_backsubstitute(first, last, c)

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c     back solve: if last cell, then generate U(isize)=rhs(isize)
c     else assume U(isize) is loaded in un pack backsub_info
c     so just use it
c     after call u(istart) will be sent to next cell
c---------------------------------------------------------------------

      include 'header.h'

      integer first, last, c, i, j, k
      integer m,n,isize,jsize,ksize,istart
      
      istart = 0
      isize = cell_size(1,c)-1
      jsize = cell_size(2,c)-end(2,c)-1      
      ksize = cell_size(3,c)-end(3,c)-1
      if (last .eq. 0) then
         do k=start(3,c),ksize
            do j=start(2,c),jsize
c---------------------------------------------------------------------
c     U(isize) uses info from previous cell if not last cell
c---------------------------------------------------------------------
               do m=1,BLOCK_SIZE
                  do n=1,BLOCK_SIZE
                     rhs(m,isize,j,k,c) = rhs(m,isize,j,k,c) 
     >                    - lhsc(m,n,isize,j,k,c)*
     >                    backsub_info(n,j,k,c)
c---------------------------------------------------------------------
c     rhs(m,isize,j,k,c) = rhs(m,isize,j,k,c) 
c     $                    - lhsc(m,n,isize,j,k,c)*rhs(n,isize+1,j,k,c)
c---------------------------------------------------------------------
                  enddo
               enddo
            enddo
         enddo
      endif
      do k=start(3,c),ksize
         do j=start(2,c),jsize
            do i=isize-1,istart,-1
               do m=1,BLOCK_SIZE
                  do n=1,BLOCK_SIZE
                     rhs(m,i,j,k,c) = rhs(m,i,j,k,c) 
     >                    - lhsc(m,n,i,j,k,c)*rhs(n,i+1,j,k,c)
                  enddo
               enddo
            enddo
         enddo
      enddo

      return
      end


c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine x_solve_cell(first,last,c)

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c     performs guaussian elimination on this cell.
c     
c     assumes that unpacking routines for non-first cells 
c     preload C' and rhs' from previous cell.
c     
c     assumed send happens outside this routine, but that
c     c'(IMAX) and rhs'(IMAX) will be sent to next cell
c---------------------------------------------------------------------

      include 'header.h'

      integer first,last,c
      integer i,j,k,isize,ksize,jsize,istart

      istart = 0
      isize = cell_size(1,c)-1
      jsize = cell_size(2,c)-end(2,c)-1
      ksize = cell_size(3,c)-end(3,c)-1

      call lhsabinit(lhsa, lhsb, isize)

      do k=start(3,c),ksize 
         do j=start(2,c),jsize

c---------------------------------------------------------------------
c     This function computes the left hand side in the xi-direction
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c     determine a (labeled f) and n jacobians for cell c
c---------------------------------------------------------------------
            do i = start(1,c)-1, cell_size(1,c) - end(1,c)

               tmp1 = rho_i(i,j,k,c)
               tmp2 = tmp1 * tmp1
               tmp3 = tmp1 * tmp2
c---------------------------------------------------------------------
c     
c---------------------------------------------------------------------
               fjac(1,1,i) = 0.0d+00
               fjac(1,2,i) = 1.0d+00
               fjac(1,3,i) = 0.0d+00
               fjac(1,4,i) = 0.0d+00
               fjac(1,5,i) = 0.0d+00

               fjac(2,1,i) = -(u(2,i,j,k,c) * tmp2 * 
     >              u(2,i,j,k,c))
     >              + c2 * qs(i,j,k,c)
               fjac(2,2,i) = ( 2.0d+00 - c2 )
     >              * ( u(2,i,j,k,c) * tmp1 )
               fjac(2,3,i) = - c2 * ( u(3,i,j,k,c) * tmp1 )
               fjac(2,4,i) = - c2 * ( u(4,i,j,k,c) * tmp1 )
               fjac(2,5,i) = c2

               fjac(3,1,i) = - ( u(2,i,j,k,c)*u(3,i,j,k,c) ) * tmp2
               fjac(3,2,i) = u(3,i,j,k,c) * tmp1
               fjac(3,3,i) = u(2,i,j,k,c) * tmp1
               fjac(3,4,i) = 0.0d+00
               fjac(3,5,i) = 0.0d+00

               fjac(4,1,i) = - ( u(2,i,j,k,c)*u(4,i,j,k,c) ) * tmp2
               fjac(4,2,i) = u(4,i,j,k,c) * tmp1
               fjac(4,3,i) = 0.0d+00
               fjac(4,4,i) = u(2,i,j,k,c) * tmp1
               fjac(4,5,i) = 0.0d+00

               fjac(5,1,i) = ( c2 * 2.0d0 * qs(i,j,k,c)
     >              - c1 * ( u(5,i,j,k,c) * tmp1 ) )
     >              * ( u(2,i,j,k,c) * tmp1 )
               fjac(5,2,i) = c1 *  u(5,i,j,k,c) * tmp1 
     >              - c2
     >              * ( u(2,i,j,k,c)*u(2,i,j,k,c) * tmp2
     >              + qs(i,j,k,c) )
               fjac(5,3,i) = - c2 * ( u(3,i,j,k,c)*u(2,i,j,k,c) )
     >              * tmp2
               fjac(5,4,i) = - c2 * ( u(4,i,j,k,c)*u(2,i,j,k,c) )
     >              * tmp2
               fjac(5,5,i) = c1 * ( u(2,i,j,k,c) * tmp1 )

               njac(1,1,i) = 0.0d+00
               njac(1,2,i) = 0.0d+00
               njac(1,3,i) = 0.0d+00
               njac(1,4,i) = 0.0d+00
               njac(1,5,i) = 0.0d+00

               njac(2,1,i) = - con43 * c3c4 * tmp2 * u(2,i,j,k,c)
               njac(2,2,i) =   con43 * c3c4 * tmp1
               njac(2,3,i) =   0.0d+00
               njac(2,4,i) =   0.0d+00
               njac(2,5,i) =   0.0d+00

               njac(3,1,i) = - c3c4 * tmp2 * u(3,i,j,k,c)
               njac(3,2,i) =   0.0d+00
               njac(3,3,i) =   c3c4 * tmp1
               njac(3,4,i) =   0.0d+00
               njac(3,5,i) =   0.0d+00

               njac(4,1,i) = - c3c4 * tmp2 * u(4,i,j,k,c)
               njac(4,2,i) =   0.0d+00 
               njac(4,3,i) =   0.0d+00
               njac(4,4,i) =   c3c4 * tmp1
               njac(4,5,i) =   0.0d+00

               njac(5,1,i) = - ( con43 * c3c4
     >              - c1345 ) * tmp3 * (u(2,i,j,k,c)**2)
     >              - ( c3c4 - c1345 ) * tmp3 * (u(3,i,j,k,c)**2)
     >              - ( c3c4 - c1345 ) * tmp3 * (u(4,i,j,k,c)**2)
     >              - c1345 * tmp2 * u(5,i,j,k,c)

               njac(5,2,i) = ( con43 * c3c4
     >              - c1345 ) * tmp2 * u(2,i,j,k,c)
               njac(5,3,i) = ( c3c4 - c1345 ) * tmp2 * u(3,i,j,k,c)
               njac(5,4,i) = ( c3c4 - c1345 ) * tmp2 * u(4,i,j,k,c)
               njac(5,5,i) = ( c1345 ) * tmp1

            enddo
c---------------------------------------------------------------------
c     now jacobians set, so form left hand side in x direction
c---------------------------------------------------------------------
            do i = start(1,c), isize - end(1,c)

               tmp1 = dt * tx1
               tmp2 = dt * tx2

               lhsa(1,1,i) = - tmp2 * fjac(1,1,i-1)
     >              - tmp1 * njac(1,1,i-1)
     >              - tmp1 * dx1 
               lhsa(1,2,i) = - tmp2 * fjac(1,2,i-1)
     >              - tmp1 * njac(1,2,i-1)
               lhsa(1,3,i) = - tmp2 * fjac(1,3,i-1)
     >              - tmp1 * njac(1,3,i-1)
               lhsa(1,4,i) = - tmp2 * fjac(1,4,i-1)
     >              - tmp1 * njac(1,4,i-1)
               lhsa(1,5,i) = - tmp2 * fjac(1,5,i-1)
     >              - tmp1 * njac(1,5,i-1)

               lhsa(2,1,i) = - tmp2 * fjac(2,1,i-1)
     >              - tmp1 * njac(2,1,i-1)
               lhsa(2,2,i) = - tmp2 * fjac(2,2,i-1)
     >              - tmp1 * njac(2,2,i-1)
     >              - tmp1 * dx2
               lhsa(2,3,i) = - tmp2 * fjac(2,3,i-1)
     >              - tmp1 * njac(2,3,i-1)
               lhsa(2,4,i) = - tmp2 * fjac(2,4,i-1)
     >              - tmp1 * njac(2,4,i-1)
               lhsa(2,5,i) = - tmp2 * fjac(2,5,i-1)
     >              - tmp1 * njac(2,5,i-1)

               lhsa(3,1,i) = - tmp2 * fjac(3,1,i-1)
     >              - tmp1 * njac(3,1,i-1)
               lhsa(3,2,i) = - tmp2 * fjac(3,2,i-1)
     >              - tmp1 * njac(3,2,i-1)
               lhsa(3,3,i) = - tmp2 * fjac(3,3,i-1)
     >              - tmp1 * njac(3,3,i-1)
     >              - tmp1 * dx3 
               lhsa(3,4,i) = - tmp2 * fjac(3,4,i-1)
     >              - tmp1 * njac(3,4,i-1)
               lhsa(3,5,i) = - tmp2 * fjac(3,5,i-1)
     >              - tmp1 * njac(3,5,i-1)

               lhsa(4,1,i) = - tmp2 * fjac(4,1,i-1)
     >              - tmp1 * njac(4,1,i-1)
               lhsa(4,2,i) = - tmp2 * fjac(4,2,i-1)
     >              - tmp1 * njac(4,2,i-1)
               lhsa(4,3,i) = - tmp2 * fjac(4,3,i-1)
     >              - tmp1 * njac(4,3,i-1)
               lhsa(4,4,i) = - tmp2 * fjac(4,4,i-1)
     >              - tmp1 * njac(4,4,i-1)
     >              - tmp1 * dx4
               lhsa(4,5,i) = - tmp2 * fjac(4,5,i-1)
     >              - tmp1 * njac(4,5,i-1)

               lhsa(5,1,i) = - tmp2 * fjac(5,1,i-1)
     >              - tmp1 * njac(5,1,i-1)
               lhsa(5,2,i) = - tmp2 * fjac(5,2,i-1)
     >              - tmp1 * njac(5,2,i-1)
               lhsa(5,3,i) = - tmp2 * fjac(5,3,i-1)
     >              - tmp1 * njac(5,3,i-1)
               lhsa(5,4,i) = - tmp2 * fjac(5,4,i-1)
     >              - tmp1 * njac(5,4,i-1)
               lhsa(5,5,i) = - tmp2 * fjac(5,5,i-1)
     >              - tmp1 * njac(5,5,i-1)
     >              - tmp1 * dx5

               lhsb(1,1,i) = 1.0d+00
     >              + tmp1 * 2.0d+00 * njac(1,1,i)
     >              + tmp1 * 2.0d+00 * dx1
               lhsb(1,2,i) = tmp1 * 2.0d+00 * njac(1,2,i)
               lhsb(1,3,i) = tmp1 * 2.0d+00 * njac(1,3,i)
               lhsb(1,4,i) = tmp1 * 2.0d+00 * njac(1,4,i)
               lhsb(1,5,i) = tmp1 * 2.0d+00 * njac(1,5,i)

               lhsb(2,1,i) = tmp1 * 2.0d+00 * njac(2,1,i)
               lhsb(2,2,i) = 1.0d+00
     >              + tmp1 * 2.0d+00 * njac(2,2,i)
     >              + tmp1 * 2.0d+00 * dx2
               lhsb(2,3,i) = tmp1 * 2.0d+00 * njac(2,3,i)
               lhsb(2,4,i) = tmp1 * 2.0d+00 * njac(2,4,i)
               lhsb(2,5,i) = tmp1 * 2.0d+00 * njac(2,5,i)

               lhsb(3,1,i) = tmp1 * 2.0d+00 * njac(3,1,i)
               lhsb(3,2,i) = tmp1 * 2.0d+00 * njac(3,2,i)
               lhsb(3,3,i) = 1.0d+00
     >              + tmp1 * 2.0d+00 * njac(3,3,i)
     >              + tmp1 * 2.0d+00 * dx3
               lhsb(3,4,i) = tmp1 * 2.0d+00 * njac(3,4,i)
               lhsb(3,5,i) = tmp1 * 2.0d+00 * njac(3,5,i)

               lhsb(4,1,i) = tmp1 * 2.0d+00 * njac(4,1,i)
               lhsb(4,2,i) = tmp1 * 2.0d+00 * njac(4,2,i)
               lhsb(4,3,i) = tmp1 * 2.0d+00 * njac(4,3,i)
               lhsb(4,4,i) = 1.0d+00
     >              + tmp1 * 2.0d+00 * njac(4,4,i)
     >              + tmp1 * 2.0d+00 * dx4
               lhsb(4,5,i) = tmp1 * 2.0d+00 * njac(4,5,i)

               lhsb(5,1,i) = tmp1 * 2.0d+00 * njac(5,1,i)
               lhsb(5,2,i) = tmp1 * 2.0d+00 * njac(5,2,i)
               lhsb(5,3,i) = tmp1 * 2.0d+00 * njac(5,3,i)
               lhsb(5,4,i) = tmp1 * 2.0d+00 * njac(5,4,i)
               lhsb(5,5,i) = 1.0d+00
     >              + tmp1 * 2.0d+00 * njac(5,5,i)
     >              + tmp1 * 2.0d+00 * dx5

               lhsc(1,1,i,j,k,c) =  tmp2 * fjac(1,1,i+1)
     >              - tmp1 * njac(1,1,i+1)
     >              - tmp1 * dx1
               lhsc(1,2,i,j,k,c) =  tmp2 * fjac(1,2,i+1)
     >              - tmp1 * njac(1,2,i+1)
               lhsc(1,3,i,j,k,c) =  tmp2 * fjac(1,3,i+1)
     >              - tmp1 * njac(1,3,i+1)
               lhsc(1,4,i,j,k,c) =  tmp2 * fjac(1,4,i+1)
     >              - tmp1 * njac(1,4,i+1)
               lhsc(1,5,i,j,k,c) =  tmp2 * fjac(1,5,i+1)
     >              - tmp1 * njac(1,5,i+1)

               lhsc(2,1,i,j,k,c) =  tmp2 * fjac(2,1,i+1)
     >              - tmp1 * njac(2,1,i+1)
               lhsc(2,2,i,j,k,c) =  tmp2 * fjac(2,2,i+1)
     >              - tmp1 * njac(2,2,i+1)
     >              - tmp1 * dx2
               lhsc(2,3,i,j,k,c) =  tmp2 * fjac(2,3,i+1)
     >              - tmp1 * njac(2,3,i+1)
               lhsc(2,4,i,j,k,c) =  tmp2 * fjac(2,4,i+1)
     >              - tmp1 * njac(2,4,i+1)
               lhsc(2,5,i,j,k,c) =  tmp2 * fjac(2,5,i+1)
     >              - tmp1 * njac(2,5,i+1)

               lhsc(3,1,i,j,k,c) =  tmp2 * fjac(3,1,i+1)
     >              - tmp1 * njac(3,1,i+1)
               lhsc(3,2,i,j,k,c) =  tmp2 * fjac(3,2,i+1)
     >              - tmp1 * njac(3,2,i+1)
               lhsc(3,3,i,j,k,c) =  tmp2 * fjac(3,3,i+1)
     >              - tmp1 * njac(3,3,i+1)
     >              - tmp1 * dx3
               lhsc(3,4,i,j,k,c) =  tmp2 * fjac(3,4,i+1)
     >              - tmp1 * njac(3,4,i+1)
               lhsc(3,5,i,j,k,c) =  tmp2 * fjac(3,5,i+1)
     >              - tmp1 * njac(3,5,i+1)

               lhsc(4,1,i,j,k,c) =  tmp2 * fjac(4,1,i+1)
     >              - tmp1 * njac(4,1,i+1)
               lhsc(4,2,i,j,k,c) =  tmp2 * fjac(4,2,i+1)
     >              - tmp1 * njac(4,2,i+1)
               lhsc(4,3,i,j,k,c) =  tmp2 * fjac(4,3,i+1)
     >              - tmp1 * njac(4,3,i+1)
               lhsc(4,4,i,j,k,c) =  tmp2 * fjac(4,4,i+1)
     >              - tmp1 * njac(4,4,i+1)
     >              - tmp1 * dx4
               lhsc(4,5,i,j,k,c) =  tmp2 * fjac(4,5,i+1)
     >              - tmp1 * njac(4,5,i+1)

               lhsc(5,1,i,j,k,c) =  tmp2 * fjac(5,1,i+1)
     >              - tmp1 * njac(5,1,i+1)
               lhsc(5,2,i,j,k,c) =  tmp2 * fjac(5,2,i+1)
     >              - tmp1 * njac(5,2,i+1)
               lhsc(5,3,i,j,k,c) =  tmp2 * fjac(5,3,i+1)
     >              - tmp1 * njac(5,3,i+1)
               lhsc(5,4,i,j,k,c) =  tmp2 * fjac(5,4,i+1)
     >              - tmp1 * njac(5,4,i+1)
               lhsc(5,5,i,j,k,c) =  tmp2 * fjac(5,5,i+1)
     >              - tmp1 * njac(5,5,i+1)
     >              - tmp1 * dx5

            enddo


c---------------------------------------------------------------------
c     outer most do loops - sweeping in i direction
c---------------------------------------------------------------------
      	    if (first .eq. 1) then 

c---------------------------------------------------------------------
c     multiply c(istart,j,k) by b_inverse and copy back to c
c     multiply rhs(istart) by b_inverse(istart) and copy to rhs
c---------------------------------------------------------------------
               call binvcrhs( lhsb(1,1,istart),
     >                        lhsc(1,1,istart,j,k,c),
     >                        rhs(1,istart,j,k,c) )

      	    endif

c---------------------------------------------------------------------
c     begin inner most do loop
c     do all the elements of the cell unless last 
c---------------------------------------------------------------------
            do i=istart+first,isize-last

c---------------------------------------------------------------------
c     rhs(i) = rhs(i) - A*rhs(i-1)
c---------------------------------------------------------------------
               call matvec_sub(lhsa(1,1,i),
     >                         rhs(1,i-1,j,k,c),rhs(1,i,j,k,c))

c---------------------------------------------------------------------
c     B(i) = B(i) - C(i-1)*A(i)
c---------------------------------------------------------------------
               call matmul_sub(lhsa(1,1,i),
     >                         lhsc(1,1,i-1,j,k,c),
     >                         lhsb(1,1,i))


c---------------------------------------------------------------------
c     multiply c(i,j,k) by b_inverse and copy back to c
c     multiply rhs(1,j,k) by b_inverse(1,j,k) and copy to rhs
c---------------------------------------------------------------------
               call binvcrhs( lhsb(1,1,i),
     >                        lhsc(1,1,i,j,k,c),
     >                        rhs(1,i,j,k,c) )

            enddo

c---------------------------------------------------------------------
c     Now finish up special cases for last cell
c---------------------------------------------------------------------
      	    if (last .eq. 1) then

c---------------------------------------------------------------------
c     rhs(isize) = rhs(isize) - A*rhs(isize-1)
c---------------------------------------------------------------------
               call matvec_sub(lhsa(1,1,isize),
     >                         rhs(1,isize-1,j,k,c),rhs(1,isize,j,k,c))

c---------------------------------------------------------------------
c     B(isize) = B(isize) - C(isize-1)*A(isize)
c---------------------------------------------------------------------
               call matmul_sub(lhsa(1,1,isize),
     >                         lhsc(1,1,isize-1,j,k,c),
     >                         lhsb(1,1,isize))

c---------------------------------------------------------------------
c     multiply rhs() by b_inverse() and copy to rhs
c---------------------------------------------------------------------
               call binvrhs( lhsb(1,1,isize),
     >                       rhs(1,isize,j,k,c) )

      	    endif
         enddo
      enddo
 1000 format (6(2x,e8.2))

      return
      end
      

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine matvec_sub(ablock,avec,bvec)

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c     subtracts bvec=bvec - ablock*avec
c---------------------------------------------------------------------

      implicit none

      double precision ablock,avec,bvec
      dimension ablock(5,5),avec(5),bvec(5)
      integer i

      do i=1,5
c---------------------------------------------------------------------
c            rhs(i,ic,jc,kc,ccell) = rhs(i,ic,jc,kc,ccell) 
c     $           - lhs(i,1,ablock,ia,ja,ka,acell)*
c---------------------------------------------------------------------
         bvec(i) = bvec(i) - ablock(i,1)*avec(1)
     >                     - ablock(i,2)*avec(2)
     >                     - ablock(i,3)*avec(3)
     >                     - ablock(i,4)*avec(4)
     >                     - ablock(i,5)*avec(5)
      enddo


      return
      end

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine matmul_sub(ablock, bblock, cblock)

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c     subtracts a(i,j,k) X b(i,j,k) from c(i,j,k)
c---------------------------------------------------------------------

      implicit none

      double precision ablock, bblock, cblock
      dimension ablock(5,5), bblock(5,5), cblock(5,5)
      integer j


      do j=1,5
         cblock(1,j) = cblock(1,j) - ablock(1,1)*bblock(1,j)
     >                             - ablock(1,2)*bblock(2,j)
     >                             - ablock(1,3)*bblock(3,j)
     >                             - ablock(1,4)*bblock(4,j)
     >                             - ablock(1,5)*bblock(5,j)
         cblock(2,j) = cblock(2,j) - ablock(2,1)*bblock(1,j)
     >                             - ablock(2,2)*bblock(2,j)
     >                             - ablock(2,3)*bblock(3,j)
     >                             - ablock(2,4)*bblock(4,j)
     >                             - ablock(2,5)*bblock(5,j)
         cblock(3,j) = cblock(3,j) - ablock(3,1)*bblock(1,j)
     >                             - ablock(3,2)*bblock(2,j)
     >                             - ablock(3,3)*bblock(3,j)
     >                             - ablock(3,4)*bblock(4,j)
     >                             - ablock(3,5)*bblock(5,j)
         cblock(4,j) = cblock(4,j) - ablock(4,1)*bblock(1,j)
     >                             - ablock(4,2)*bblock(2,j)
     >                             - ablock(4,3)*bblock(3,j)
     >                             - ablock(4,4)*bblock(4,j)
     >                             - ablock(4,5)*bblock(5,j)
         cblock(5,j) = cblock(5,j) - ablock(5,1)*bblock(1,j)
     >                             - ablock(5,2)*bblock(2,j)
     >                             - ablock(5,3)*bblock(3,j)
     >                             - ablock(5,4)*bblock(4,j)
     >                             - ablock(5,5)*bblock(5,j)
      enddo

              
      return
      end



c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine binvcrhs( lhs,c,r )

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c     
c---------------------------------------------------------------------

      implicit none

      double precision pivot, coeff, lhs
      dimension lhs(5,5)
      double precision c(5,5), r(5)

c---------------------------------------------------------------------
c     
c---------------------------------------------------------------------

      pivot = 1.00d0/lhs(1,1)
      lhs(1,2) = lhs(1,2)*pivot
      lhs(1,3) = lhs(1,3)*pivot
      lhs(1,4) = lhs(1,4)*pivot
      lhs(1,5) = lhs(1,5)*pivot
      c(1,1) = c(1,1)*pivot
      c(1,2) = c(1,2)*pivot
      c(1,3) = c(1,3)*pivot
      c(1,4) = c(1,4)*pivot
      c(1,5) = c(1,5)*pivot
      r(1)   = r(1)  *pivot

      coeff = lhs(2,1)
      lhs(2,2)= lhs(2,2) - coeff*lhs(1,2)
      lhs(2,3)= lhs(2,3) - coeff*lhs(1,3)
      lhs(2,4)= lhs(2,4) - coeff*lhs(1,4)
      lhs(2,5)= lhs(2,5) - coeff*lhs(1,5)
      c(2,1) = c(2,1) - coeff*c(1,1)
      c(2,2) = c(2,2) - coeff*c(1,2)
      c(2,3) = c(2,3) - coeff*c(1,3)
      c(2,4) = c(2,4) - coeff*c(1,4)
      c(2,5) = c(2,5) - coeff*c(1,5)
      r(2)   = r(2)   - coeff*r(1)

      coeff = lhs(3,1)
      lhs(3,2)= lhs(3,2) - coeff*lhs(1,2)
      lhs(3,3)= lhs(3,3) - coeff*lhs(1,3)
      lhs(3,4)= lhs(3,4) - coeff*lhs(1,4)
      lhs(3,5)= lhs(3,5) - coeff*lhs(1,5)
      c(3,1) = c(3,1) - coeff*c(1,1)
      c(3,2) = c(3,2) - coeff*c(1,2)
      c(3,3) = c(3,3) - coeff*c(1,3)
      c(3,4) = c(3,4) - coeff*c(1,4)
      c(3,5) = c(3,5) - coeff*c(1,5)
      r(3)   = r(3)   - coeff*r(1)

      coeff = lhs(4,1)
      lhs(4,2)= lhs(4,2) - coeff*lhs(1,2)
      lhs(4,3)= lhs(4,3) - coeff*lhs(1,3)
      lhs(4,4)= lhs(4,4) - coeff*lhs(1,4)
      lhs(4,5)= lhs(4,5) - coeff*lhs(1,5)
      c(4,1) = c(4,1) - coeff*c(1,1)
      c(4,2) = c(4,2) - coeff*c(1,2)
      c(4,3) = c(4,3) - coeff*c(1,3)
      c(4,4) = c(4,4) - coeff*c(1,4)
      c(4,5) = c(4,5) - coeff*c(1,5)
      r(4)   = r(4)   - coeff*r(1)

      coeff = lhs(5,1)
      lhs(5,2)= lhs(5,2) - coeff*lhs(1,2)
      lhs(5,3)= lhs(5,3) - coeff*lhs(1,3)
      lhs(5,4)= lhs(5,4) - coeff*lhs(1,4)
      lhs(5,5)= lhs(5,5) - coeff*lhs(1,5)
      c(5,1) = c(5,1) - coeff*c(1,1)
      c(5,2) = c(5,2) - coeff*c(1,2)
      c(5,3) = c(5,3) - coeff*c(1,3)
      c(5,4) = c(5,4) - coeff*c(1,4)
      c(5,5) = c(5,5) - coeff*c(1,5)
      r(5)   = r(5)   - coeff*r(1)


      pivot = 1.00d0/lhs(2,2)
      lhs(2,3) = lhs(2,3)*pivot
      lhs(2,4) = lhs(2,4)*pivot
      lhs(2,5) = lhs(2,5)*pivot
      c(2,1) = c(2,1)*pivot
      c(2,2) = c(2,2)*pivot
      c(2,3) = c(2,3)*pivot
      c(2,4) = c(2,4)*pivot
      c(2,5) = c(2,5)*pivot
      r(2)   = r(2)  *pivot

      coeff = lhs(1,2)
      lhs(1,3)= lhs(1,3) - coeff*lhs(2,3)
      lhs(1,4)= lhs(1,4) - coeff*lhs(2,4)
      lhs(1,5)= lhs(1,5) - coeff*lhs(2,5)
      c(1,1) = c(1,1) - coeff*c(2,1)
      c(1,2) = c(1,2) - coeff*c(2,2)
      c(1,3) = c(1,3) - coeff*c(2,3)
      c(1,4) = c(1,4) - coeff*c(2,4)
      c(1,5) = c(1,5) - coeff*c(2,5)
      r(1)   = r(1)   - coeff*r(2)

      coeff = lhs(3,2)
      lhs(3,3)= lhs(3,3) - coeff*lhs(2,3)
      lhs(3,4)= lhs(3,4) - coeff*lhs(2,4)
      lhs(3,5)= lhs(3,5) - coeff*lhs(2,5)
      c(3,1) = c(3,1) - coeff*c(2,1)
      c(3,2) = c(3,2) - coeff*c(2,2)
      c(3,3) = c(3,3) - coeff*c(2,3)
      c(3,4) = c(3,4) - coeff*c(2,4)
      c(3,5) = c(3,5) - coeff*c(2,5)
      r(3)   = r(3)   - coeff*r(2)

      coeff = lhs(4,2)
      lhs(4,3)= lhs(4,3) - coeff*lhs(2,3)
      lhs(4,4)= lhs(4,4) - coeff*lhs(2,4)
      lhs(4,5)= lhs(4,5) - coeff*lhs(2,5)
      c(4,1) = c(4,1) - coeff*c(2,1)
      c(4,2) = c(4,2) - coeff*c(2,2)
      c(4,3) = c(4,3) - coeff*c(2,3)
      c(4,4) = c(4,4) - coeff*c(2,4)
      c(4,5) = c(4,5) - coeff*c(2,5)
      r(4)   = r(4)   - coeff*r(2)

      coeff = lhs(5,2)
      lhs(5,3)= lhs(5,3) - coeff*lhs(2,3)
      lhs(5,4)= lhs(5,4) - coeff*lhs(2,4)
      lhs(5,5)= lhs(5,5) - coeff*lhs(2,5)
      c(5,1) = c(5,1) - coeff*c(2,1)
      c(5,2) = c(5,2) - coeff*c(2,2)
      c(5,3) = c(5,3) - coeff*c(2,3)
      c(5,4) = c(5,4) - coeff*c(2,4)
      c(5,5) = c(5,5) - coeff*c(2,5)
      r(5)   = r(5)   - coeff*r(2)


      pivot = 1.00d0/lhs(3,3)
      lhs(3,4) = lhs(3,4)*pivot
      lhs(3,5) = lhs(3,5)*pivot
      c(3,1) = c(3,1)*pivot
      c(3,2) = c(3,2)*pivot
      c(3,3) = c(3,3)*pivot
      c(3,4) = c(3,4)*pivot
      c(3,5) = c(3,5)*pivot
      r(3)   = r(3)  *pivot

      coeff = lhs(1,3)
      lhs(1,4)= lhs(1,4) - coeff*lhs(3,4)
      lhs(1,5)= lhs(1,5) - coeff*lhs(3,5)
      c(1,1) = c(1,1) - coeff*c(3,1)
      c(1,2) = c(1,2) - coeff*c(3,2)
      c(1,3) = c(1,3) - coeff*c(3,3)
      c(1,4) = c(1,4) - coeff*c(3,4)
      c(1,5) = c(1,5) - coeff*c(3,5)
      r(1)   = r(1)   - coeff*r(3)

      coeff = lhs(2,3)
      lhs(2,4)= lhs(2,4) - coeff*lhs(3,4)
      lhs(2,5)= lhs(2,5) - coeff*lhs(3,5)
      c(2,1) = c(2,1) - coeff*c(3,1)
      c(2,2) = c(2,2) - coeff*c(3,2)
      c(2,3) = c(2,3) - coeff*c(3,3)
      c(2,4) = c(2,4) - coeff*c(3,4)
      c(2,5) = c(2,5) - coeff*c(3,5)
      r(2)   = r(2)   - coeff*r(3)

      coeff = lhs(4,3)
      lhs(4,4)= lhs(4,4) - coeff*lhs(3,4)
      lhs(4,5)= lhs(4,5) - coeff*lhs(3,5)
      c(4,1) = c(4,1) - coeff*c(3,1)
      c(4,2) = c(4,2) - coeff*c(3,2)
      c(4,3) = c(4,3) - coeff*c(3,3)
      c(4,4) = c(4,4) - coeff*c(3,4)
      c(4,5) = c(4,5) - coeff*c(3,5)
      r(4)   = r(4)   - coeff*r(3)

      coeff = lhs(5,3)
      lhs(5,4)= lhs(5,4) - coeff*lhs(3,4)
      lhs(5,5)= lhs(5,5) - coeff*lhs(3,5)
      c(5,1) = c(5,1) - coeff*c(3,1)
      c(5,2) = c(5,2) - coeff*c(3,2)
      c(5,3) = c(5,3) - coeff*c(3,3)
      c(5,4) = c(5,4) - coeff*c(3,4)
      c(5,5) = c(5,5) - coeff*c(3,5)
      r(5)   = r(5)   - coeff*r(3)


      pivot = 1.00d0/lhs(4,4)
      lhs(4,5) = lhs(4,5)*pivot
      c(4,1) = c(4,1)*pivot
      c(4,2) = c(4,2)*pivot
      c(4,3) = c(4,3)*pivot
      c(4,4) = c(4,4)*pivot
      c(4,5) = c(4,5)*pivot
      r(4)   = r(4)  *pivot

      coeff = lhs(1,4)
      lhs(1,5)= lhs(1,5) - coeff*lhs(4,5)
      c(1,1) = c(1,1) - coeff*c(4,1)
      c(1,2) = c(1,2) - coeff*c(4,2)
      c(1,3) = c(1,3) - coeff*c(4,3)
      c(1,4) = c(1,4) - coeff*c(4,4)
      c(1,5) = c(1,5) - coeff*c(4,5)
      r(1)   = r(1)   - coeff*r(4)

      coeff = lhs(2,4)
      lhs(2,5)= lhs(2,5) - coeff*lhs(4,5)
      c(2,1) = c(2,1) - coeff*c(4,1)
      c(2,2) = c(2,2) - coeff*c(4,2)
      c(2,3) = c(2,3) - coeff*c(4,3)
      c(2,4) = c(2,4) - coeff*c(4,4)
      c(2,5) = c(2,5) - coeff*c(4,5)
      r(2)   = r(2)   - coeff*r(4)

      coeff = lhs(3,4)
      lhs(3,5)= lhs(3,5) - coeff*lhs(4,5)
      c(3,1) = c(3,1) - coeff*c(4,1)
      c(3,2) = c(3,2) - coeff*c(4,2)
      c(3,3) = c(3,3) - coeff*c(4,3)
      c(3,4) = c(3,4) - coeff*c(4,4)
      c(3,5) = c(3,5) - coeff*c(4,5)
      r(3)   = r(3)   - coeff*r(4)

      coeff = lhs(5,4)
      lhs(5,5)= lhs(5,5) - coeff*lhs(4,5)
      c(5,1) = c(5,1) - coeff*c(4,1)
      c(5,2) = c(5,2) - coeff*c(4,2)
      c(5,3) = c(5,3) - coeff*c(4,3)
      c(5,4) = c(5,4) - coeff*c(4,4)
      c(5,5) = c(5,5) - coeff*c(4,5)
      r(5)   = r(5)   - coeff*r(4)


      pivot = 1.00d0/lhs(5,5)
      c(5,1) = c(5,1)*pivot
      c(5,2) = c(5,2)*pivot
      c(5,3) = c(5,3)*pivot
      c(5,4) = c(5,4)*pivot
      c(5,5) = c(5,5)*pivot
      r(5)   = r(5)  *pivot

      coeff = lhs(1,5)
      c(1,1) = c(1,1) - coeff*c(5,1)
      c(1,2) = c(1,2) - coeff*c(5,2)
      c(1,3) = c(1,3) - coeff*c(5,3)
      c(1,4) = c(1,4) - coeff*c(5,4)
      c(1,5) = c(1,5) - coeff*c(5,5)
      r(1)   = r(1)   - coeff*r(5)

      coeff = lhs(2,5)
      c(2,1) = c(2,1) - coeff*c(5,1)
      c(2,2) = c(2,2) - coeff*c(5,2)
      c(2,3) = c(2,3) - coeff*c(5,3)
      c(2,4) = c(2,4) - coeff*c(5,4)
      c(2,5) = c(2,5) - coeff*c(5,5)
      r(2)   = r(2)   - coeff*r(5)

      coeff = lhs(3,5)
      c(3,1) = c(3,1) - coeff*c(5,1)
      c(3,2) = c(3,2) - coeff*c(5,2)
      c(3,3) = c(3,3) - coeff*c(5,3)
      c(3,4) = c(3,4) - coeff*c(5,4)
      c(3,5) = c(3,5) - coeff*c(5,5)
      r(3)   = r(3)   - coeff*r(5)

      coeff = lhs(4,5)
      c(4,1) = c(4,1) - coeff*c(5,1)
      c(4,2) = c(4,2) - coeff*c(5,2)
      c(4,3) = c(4,3) - coeff*c(5,3)
      c(4,4) = c(4,4) - coeff*c(5,4)
      c(4,5) = c(4,5) - coeff*c(5,5)
      r(4)   = r(4)   - coeff*r(5)


      return
      end



c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine binvrhs( lhs,r )

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c     
c---------------------------------------------------------------------

      implicit none

      double precision pivot, coeff, lhs
      dimension lhs(5,5)
      double precision r(5)

c---------------------------------------------------------------------
c     
c---------------------------------------------------------------------


      pivot = 1.00d0/lhs(1,1)
      lhs(1,2) = lhs(1,2)*pivot
      lhs(1,3) = lhs(1,3)*pivot
      lhs(1,4) = lhs(1,4)*pivot
      lhs(1,5) = lhs(1,5)*pivot
      r(1)   = r(1)  *pivot

      coeff = lhs(2,1)
      lhs(2,2)= lhs(2,2) - coeff*lhs(1,2)
      lhs(2,3)= lhs(2,3) - coeff*lhs(1,3)
      lhs(2,4)= lhs(2,4) - coeff*lhs(1,4)
      lhs(2,5)= lhs(2,5) - coeff*lhs(1,5)
      r(2)   = r(2)   - coeff*r(1)

      coeff = lhs(3,1)
      lhs(3,2)= lhs(3,2) - coeff*lhs(1,2)
      lhs(3,3)= lhs(3,3) - coeff*lhs(1,3)
      lhs(3,4)= lhs(3,4) - coeff*lhs(1,4)
      lhs(3,5)= lhs(3,5) - coeff*lhs(1,5)
      r(3)   = r(3)   - coeff*r(1)

      coeff = lhs(4,1)
      lhs(4,2)= lhs(4,2) - coeff*lhs(1,2)
      lhs(4,3)= lhs(4,3) - coeff*lhs(1,3)
      lhs(4,4)= lhs(4,4) - coeff*lhs(1,4)
      lhs(4,5)= lhs(4,5) - coeff*lhs(1,5)
      r(4)   = r(4)   - coeff*r(1)

      coeff = lhs(5,1)
      lhs(5,2)= lhs(5,2) - coeff*lhs(1,2)
      lhs(5,3)= lhs(5,3) - coeff*lhs(1,3)
      lhs(5,4)= lhs(5,4) - coeff*lhs(1,4)
      lhs(5,5)= lhs(5,5) - coeff*lhs(1,5)
      r(5)   = r(5)   - coeff*r(1)


      pivot = 1.00d0/lhs(2,2)
      lhs(2,3) = lhs(2,3)*pivot
      lhs(2,4) = lhs(2,4)*pivot
      lhs(2,5) = lhs(2,5)*pivot
      r(2)   = r(2)  *pivot

      coeff = lhs(1,2)
      lhs(1,3)= lhs(1,3) - coeff*lhs(2,3)
      lhs(1,4)= lhs(1,4) - coeff*lhs(2,4)
      lhs(1,5)= lhs(1,5) - coeff*lhs(2,5)
      r(1)   = r(1)   - coeff*r(2)

      coeff = lhs(3,2)
      lhs(3,3)= lhs(3,3) - coeff*lhs(2,3)
      lhs(3,4)= lhs(3,4) - coeff*lhs(2,4)
      lhs(3,5)= lhs(3,5) - coeff*lhs(2,5)
      r(3)   = r(3)   - coeff*r(2)

      coeff = lhs(4,2)
      lhs(4,3)= lhs(4,3) - coeff*lhs(2,3)
      lhs(4,4)= lhs(4,4) - coeff*lhs(2,4)
      lhs(4,5)= lhs(4,5) - coeff*lhs(2,5)
      r(4)   = r(4)   - coeff*r(2)

      coeff = lhs(5,2)
      lhs(5,3)= lhs(5,3) - coeff*lhs(2,3)
      lhs(5,4)= lhs(5,4) - coeff*lhs(2,4)
      lhs(5,5)= lhs(5,5) - coeff*lhs(2,5)
      r(5)   = r(5)   - coeff*r(2)


      pivot = 1.00d0/lhs(3,3)
      lhs(3,4) = lhs(3,4)*pivot
      lhs(3,5) = lhs(3,5)*pivot
      r(3)   = r(3)  *pivot

      coeff = lhs(1,3)
      lhs(1,4)= lhs(1,4) - coeff*lhs(3,4)
      lhs(1,5)= lhs(1,5) - coeff*lhs(3,5)
      r(1)   = r(1)   - coeff*r(3)

      coeff = lhs(2,3)
      lhs(2,4)= lhs(2,4) - coeff*lhs(3,4)
      lhs(2,5)= lhs(2,5) - coeff*lhs(3,5)
      r(2)   = r(2)   - coeff*r(3)

      coeff = lhs(4,3)
      lhs(4,4)= lhs(4,4) - coeff*lhs(3,4)
      lhs(4,5)= lhs(4,5) - coeff*lhs(3,5)
      r(4)   = r(4)   - coeff*r(3)

      coeff = lhs(5,3)
      lhs(5,4)= lhs(5,4) - coeff*lhs(3,4)
      lhs(5,5)= lhs(5,5) - coeff*lhs(3,5)
      r(5)   = r(5)   - coeff*r(3)


      pivot = 1.00d0/lhs(4,4)
      lhs(4,5) = lhs(4,5)*pivot
      r(4)   = r(4)  *pivot

      coeff = lhs(1,4)
      lhs(1,5)= lhs(1,5) - coeff*lhs(4,5)
      r(1)   = r(1)   - coeff*r(4)

      coeff = lhs(2,4)
      lhs(2,5)= lhs(2,5) - coeff*lhs(4,5)
      r(2)   = r(2)   - coeff*r(4)

      coeff = lhs(3,4)
      lhs(3,5)= lhs(3,5) - coeff*lhs(4,5)
      r(3)   = r(3)   - coeff*r(4)

      coeff = lhs(5,4)
      lhs(5,5)= lhs(5,5) - coeff*lhs(4,5)
      r(5)   = r(5)   - coeff*r(4)


      pivot = 1.00d0/lhs(5,5)
      r(5)   = r(5)  *pivot

      coeff = lhs(1,5)
      r(1)   = r(1)   - coeff*r(5)

      coeff = lhs(2,5)
      r(2)   = r(2)   - coeff*r(5)

      coeff = lhs(3,5)
      r(3)   = r(3)   - coeff*r(5)

      coeff = lhs(4,5)
      r(4)   = r(4)   - coeff*r(5)


      return
      end




