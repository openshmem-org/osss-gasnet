//
// Copyright (c) 2011, 2012
//   University of Houston System and Oak Ridge National Laboratory.
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// o Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// 
// o Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// 
// o Neither the name of the University of Houston System, Oak Ridge
//   National Laboratory nor the names of its contributors may be used to
//   endorse or promote products derived from this software without specific
//   prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//




//****************************************************************************80
//
//    This program uses SHMEM to do a Buffon-Laplace simulation in parallel.
//
//    It's a rewrite of the C++ MPI program at
//
//        http://people.sc.fsu.edu/~jburkardt/cpp_src/mpi/buffon_laplace.C
//
//    which contains original licensing and author information
//
//****************************************************************************80

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;

#include <shmem.h>

int main ( int argc, char *argv[] );
int buffon_laplace_simulate ( double a, double b, double l, int trial_num );
double r8_abs ( double x );
double r8_huge ( void );

//
// symmetric variables for reduction
//
int pWrk[_SHMEM_REDUCE_SYNC_SIZE];
long pSync[_SHMEM_BCAST_SYNC_SIZE];

int hit_total;
int hit_num;

int main ( int argc, char *argv[] )
{
  double a = 1.0;
  double b = 1.0;
  double l = 1.0;
  int master = 0;
  double pdf_estimate;
  double pi = 3.141592653589793238462643;
  double pi_error;
  double pi_estimate;
  int process_num;
  int process_rank;
  double random_value;
  int seed;
  int trial_num = 100000;
  int trial_total;
  //
  //  Initialize SHMEM.
  //
  start_pes(0);
  //
  //  Get the number of processes.
  //
  process_num = _num_pes();
  //
  //  Get the rank of this process.
  //
  process_rank = _my_pe();
  //
  //  The master process prints a message.
  //
  if ( process_rank == master ) 
    {
      cout << "\n";
      cout << "BUFFON_LAPLACE - Master process:\n";
      cout << "  C++ version\n";
      cout << "\n";
      cout << "  A SHMEM example program to estimate PI\n";
      cout << "  using the Buffon-Laplace needle experiment.\n";
      cout << "  On a grid of cells of  width A and height B,\n";
      cout << "  a needle of length L is dropped at random.\n";
      cout << "  We count the number of times it crosses\n";
      cout << "  at least one grid line, and use this to estimate \n";
      cout << "  the value of PI.\n";
      cout << "\n";
      cout << "  The number of processes is " << process_num << "\n";
      cout << "\n";
      cout << "  Cell width A =    " << a << "\n";
      cout << "  Cell height B =   " << b << "\n";
      cout << "  Needle length L = " << l << "\n";
    }
  //
  // added barrier here to force output sequence
  //
  shmem_barrier_all();
  //
  //  Each process sets a random number seed.
  //
  seed = 123456789 + process_rank * 100;
  srand ( seed );
  //
  //  Just to make sure that we're all doing different things, have each
  //  process print out its rank, seed value, and a first test random value.
  //
  random_value  = ( double ) rand ( ) / ( double ) RAND_MAX;

  cout << "  " << setw(8)  << process_rank
       << "  " << setw(12) << seed
       << "  " << setw(14) << random_value << "\n";
  //
  //  Each process now carries out TRIAL_NUM trials, and then
  //  sends the value back to the master process.
  //
  hit_num = buffon_laplace_simulate ( a, b, l, trial_num );

  //
  // initialize sync buffer for reduction
  //
  for (int i = 0; i < _SHMEM_BCAST_SYNC_SIZE; i += 1) {
    pSync[i] = _SHMEM_SYNC_VALUE;
  }
  shmem_barrier_all();

  shmem_int_sum_to_all(&hit_total, &hit_num, 1, 0, 0, process_num, pWrk, pSync);

  //
  //  The master process can now estimate PI.
  //
  if ( process_rank == master )
    {
      trial_total = trial_num * process_num;

      pdf_estimate = ( double ) ( hit_total ) / ( double ) ( trial_total );

      if ( hit_total == 0 )
	{
	  pi_estimate = r8_huge ( );
	}
      else
	{
	  pi_estimate = l * ( 2.0 * ( a + b ) - l ) / ( a * b * pdf_estimate );
	}

      pi_error = r8_abs ( pi - pi_estimate );

      cout << "\n";
      cout <<
	"    Trials      Hits    Estimated PDF       Estimated Pi        Error\n";
      cout << "\n";
      cout << "  " << setw(8) << trial_total
	   << "  " << setw(8) << hit_total
	   << "  " << setw(16) << pdf_estimate
	   << "  " << setw(16) << pi_estimate
	   << "  " << setw(16) << pi_error << "\n";
    }
  //
  //  Shut down
  //
  if ( process_rank == master )
    {
      cout << "\n";
      cout << "BUFFON_LAPLACE - Master process:\n";
      cout << "  Normal end of execution.\n";
    }

  return 0;
}

int buffon_laplace_simulate ( double a, double b, double l, int trial_num )
{
  double angle;
  int hits;
  double pi = 3.141592653589793238462643;
  int trial;
  double x1;
  double x2;
  double y1;
  double y2;

  hits = 0;

  for ( trial = 1; trial <= trial_num; trial++ )
    {
      //
      //  Randomly choose the location of the eye of the needle in [0,0]x[A,B],
      //  and the angle the needle makes.
      //
      x1 = a * ( double ) rand ( ) / ( double ) RAND_MAX;
      y1 = b * ( double ) rand ( ) / ( double ) RAND_MAX;
      angle = 2.0 * pi * ( double ) rand ( ) / ( double ) RAND_MAX;
      //
      //  Compute the location of the point of the needle.
      //
      x2 = x1 + l * cos ( angle );
      y2 = y1 + l * sin ( angle );
      //
      //  Count the end locations that lie outside the cell.
      //
      if ( x2 <= 0.0 || a <= x2 || y2 <= 0.0 || b <= y2 )
	{
	  hits = hits + 1;
	}
    }
  return hits;
}

double r8_abs ( double x )
{
  if ( 0.0 <= x )
    {
      return x;
    } 
  else
    {
      return ( -x );
    }
}

double r8_huge ( )
{
  return 1.0E+30;
}
