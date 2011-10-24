
c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine setup_btio

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      include 'header.h'
      include 'mpinpb.h'

      character*(128) newfilenm

      if (node .lt. 10) then
          write (newfilenm, 996) filenm,node
      else if (node .lt. 100) then
          write (newfilenm, 997) filenm,node
      else if (node .lt. 1000) then
          write (newfilenm, 998) filenm,node
      else
          print *, 'error generating file names (> 1000 nodes)'
          stop
      endif

996   format (a,'.00',1i1)
997   format (a,'.0',1i2)
998   format (a,'.',1i3)

      open (unit=99, file=newfilenm, form='unformatted',
     $       status='unknown')

      return
      end

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine output_timestep

c---------------------------------------------------------------------
c---------------------------------------------------------------------
      include 'header.h'
      include 'mpinpb.h'

      integer ix, iio, jio, kio, cio, aio

      do cio=1,ncells
          write(99)
     $         ((((u(aio,ix, jio,kio,cio),aio=1,5),
     $             ix=0, cell_size(1,cio)-1),
     $             jio=0, cell_size(2,cio)-1),
     $             kio=0, cell_size(3,cio)-1)
      enddo

      return
      end

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine btio_cleanup

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      close(unit=99)

      return
      end

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine accumulate_norms(xce_acc)

c---------------------------------------------------------------------
c---------------------------------------------------------------------

      double precision xce_acc(*)

      return
      end
