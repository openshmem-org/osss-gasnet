c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine error_norm(rms)

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c     this function computes the norm of the difference between the
c     computed solution and the exact solution
c---------------------------------------------------------------------

      include 'header.h'
c      X-1
       include 'mpp/shmem.fh'

      integer c, i, j, k, m, ii, jj, kk, d, error
c      double precision xi, eta, zeta, u_exact(5), rms(5), rms_work(5),
c     >     add
      double precision xi, eta, zeta, u_exact(5), rms(5),
     >     add
c     X-1
      double precision, save:: rms_work(5)

c     X-1
      integer, dimension(SHMEM_BCAST_SYNC_SIZE), save :: psync1
      double precision, dimension(SHMEM_REDUCE_MIN_WRKDATA_SIZE),
     > save :: pwrk1

      psync1 = SHMEM_SYNC_VALUE
      call shmem_barrier_all();

      do m = 1, 5 
         rms_work(m) = 0.0d0
      enddo

      do c = 1, ncells
         kk = 0
         do k = cell_low(3,c), cell_high(3,c)
            zeta = dble(k) * dnzm1
            jj = 0
            do j = cell_low(2,c), cell_high(2,c)
               eta = dble(j) * dnym1
               ii = 0
               do i = cell_low(1,c), cell_high(1,c)
                  xi = dble(i) * dnxm1
                  call exact_solution(xi, eta, zeta, u_exact)

                  do m = 1, 5
                     add = u(m,ii,jj,kk,c)-u_exact(m)
                     rms_work(m) = rms_work(m) + add*add
                  enddo
                  ii = ii + 1
               enddo
               jj = jj + 1
            enddo
            kk = kk + 1
         enddo
      enddo

c      X-1
c      call mpi_allreduce(rms_work, rms, 5, 
c     >                   dp_type, MPI_SUM, MPI_COMM_WORLD, error)
      call shmem_real8_sum_to_all(rms,rms_work,5,0,0,
     >                            no_nodes,pwrk1,psync1)

      do m = 1, 5
         do d = 1, 3
            rms(m) = rms(m) / dble(grid_points(d)-2)
         enddo
         rms(m) = dsqrt(rms(m))
      enddo

      return
      end


c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine rhs_norm(rms)

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      include 'header.h'
c      X-1
       include 'mpp/shmem.fh'

      integer c, i, j, k, d, m, error
c      X-1
c      double precision rms(5), rms_work(5), add
      double precision add
      double precision rms(5)
      double precision, save:: rms_work(5)
c     X-1
      integer, dimension(SHMEM_BCAST_SYNC_SIZE), save :: psync1
      double precision, dimension(SHMEM_REDUCE_MIN_WRKDATA_SIZE),
     > save :: pwrk1

      psync1 = SHMEM_SYNC_VALUE
      call shmem_barrier_all();

      do m = 1, 5
         rms_work(m) = 0.0d0
      enddo 

      do c = 1, ncells
         do k = start(3,c), cell_size(3,c)-end(3,c)-1
            do j = start(2,c), cell_size(2,c)-end(2,c)-1
               do i = start(1,c), cell_size(1,c)-end(1,c)-1
                  do m = 1, 5
                     add = rhs(m,i,j,k,c)
                     rms_work(m) = rms_work(m) + add*add
                  enddo 
               enddo 
            enddo 
         enddo 
      enddo 

c      X-1
c      call mpi_allreduce(rms_work, rms, 5, 
c     >                   dp_type, MPI_SUM, MPI_COMM_WORLD, error)
      call shmem_real8_sum_to_all(rms,rms_work,5,0,0,
     >                            no_nodes,pwrk1,psync1)

      do m = 1, 5
         do d = 1, 3
            rms(m) = rms(m) / dble(grid_points(d)-2)
         enddo 
         rms(m) = dsqrt(rms(m))
      enddo 

      return
      end

