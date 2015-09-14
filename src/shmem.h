/* Emacs: -*- mode: c -*- */
/*
 *
 * Copyright (c) 2011 - 2015
 *   University of Houston System and UT-Battelle, LLC.
 * Copyright (c) 2009 - 2015
 *   Silicon Graphics International Corp.  SHMEM is copyrighted
 *   by Silicon Graphics International Corp. (SGI) The OpenSHMEM API
 *   (shmem) is released by Open Source Software Solutions, Inc., under an
 *   agreement with Silicon Graphics International Corp. (SGI).
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * o Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * o Neither the name of the University of Houston System,
 *   UT-Battelle, LLC. nor the names of its contributors may be used to
 *   endorse or promote products derived from this software without specific
 *   prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */



#ifndef _SHMEM_H
#define _SHMEM_H 1

/*
 * for handling the "I" (upper-case eye) macro for complex numbers
 *
 * and see end of file for tidy-up
 */
#ifdef I
# define shmemi_h_I_already_defined__
#endif  /* I */


#include <sys/types.h>
#include <stddef.h>               /* ptrdiff_t */

/*
 * C and C++ do complex numbers differently
 *
 */

#ifdef __cplusplus
# include <complex>
# define COMPLEXIFY(T) std::complex<T>
#else  /* _cplusplus */
# include <complex.h>
# define COMPLEXIFY(T) T complex
#endif  /* __cplusplus */

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

    /*
     * not all compilers support this annotation
     *
     */
#if defined(__GNUC__) || defined(__PGIC__)                              \
    || defined(__INTEL_COMPILER) || defined(__OPEN64__) || defined(__OPENUH__)
# define _WUR __attribute__((__warn_unused_result__))
#else
# define _WUR
#endif

    /*
     * TODO: need better detection
     */
#if defined(__INTEL_COMPILER) || defined(__clang__)

# define _DEPRECATED_BY(...)                                    \
    __attribute__((deprecated("use " #__VA_ARGS__ " instead")))
# define _DEPRECATED                            \
    __attribute__((deprecated))

#elif defined(__OPEN64__)

    /* not supported */

# define _DEPRECATED_BY(...)
# define _DEPRECATED

#elif defined(__GNUC__)

#define _DEPRECATED                             \
    __attribute__((deprecated))

    /* GCC has extended attribute syntax from 4.5 onward */

# if (__GNUC__ >= 5) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)
#  define _DEPRECATED_BY(...)                                   \
    __attribute__((deprecated("use " #__VA_ARGS__ " instead")))
#else
# define _DEPRECATED_BY(...)                    \
    _DEPRECATED
#endif

#else

    /* fallback */

# define _DEPRECATED_BY(...)
# define _DEPRECATED

#endif  /* compiler deprecation check */

    /*
     * start/stop & query
     */

/**
 * @brief initializes the OpenSHMEM environment on the calling PE.
 *
 * @section Synopsis
 *
 * @subsection c C/C++
 @code
 void start_pes (int npes);
 @endcode
 *
 * @subsection f Fortran
 @code
 INTEGER npes

 CALL START_PES (npes)
 @endcode
 *
 * @param npes the number of PEs participating in the program.  This
 * is ignored and should be set to 0.
 *
 * @section Effect
 *
 * Initializes the OpenSHMEM environment on the calling PE.
 *
 * @return None.
 *
 * @deprecated by \ref shmem_init()
 *
 */
      void start_pes (int npes)
          _DEPRECATED_BY(shmem_init);

    /**
     * @brief initializes the OpenSHMEM environment on the calling PE.
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_init (void);
     @endcode
     *
     * @subsection f Fortran
     @code
     CALL SHMEM_INIT ()
     @endcode
     *
     * @section Effect
     *
     * Initializes the OpenSHMEM environment on the calling PE.
     *
     * @return None.
     *
     */
    void shmem_init (void);

    /**
     * @brief finalizes the OpenSHMEM environment on the calling PE.
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_finalize (void);
     @endcode
     *
     * @subsection f Fortran
     @code
     CALL SHMEM_FINALIZE ()
     @endcode
     *
     * @section Effect
     *
     * A collective finalization of the OpenSHMEM environment on the
     * calling PE.  After a finalize call, no further OpenSHMEM calls
     * are permitted.  Any subsequent use has undefined effects.
     *
     * @return None.
     *
     */
    void shmem_finalize (void);

    /**
     * @brief causes immediate exit from the OpenSHMEM program on all PEs.
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_global_exit (int status);
     @endcode
     *
     * @subsection f Fortran
     @code
     INTEGER STATUS

     CALL SHMEM_FINALIZE (STATUS)
     @endcode
     *
     * @section Effect
     *
     * Called on 1 or more PEs, Causes immediate termination of the
     * program on all PEs.  Pending communication is flushed, files are
     * closed.  "status" allows the call to pass back information to the
     * execution environment.
     *
     * @return None.
     *
     */
    void shmem_global_exit (int status);

    /**
     * @brief returns the "rank" or identity of the calling PE
     *
     * @deprecated by \ref shmem_my_pe ()
     *
     */
    int _my_pe (void)
        _WUR _DEPRECATED_BY(shmem_my_pe);

    /**
     * @brief returns the "rank" or identity of the calling PE
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     int shmem_my_pe (void);
     @endcode
     *
     * @subsection f Fortran
     @code
     INTEGER I

     I = SHMEM_MY_PE ()
     @endcode
     *
     * @section Effect
     *
     * None.
     *
     * @return Rank of calling PE
     *
     */
    int shmem_my_pe (void) _WUR;

    /**
     * @brief These routines return the number of PEs in the program
     *
     * @deprecated by shmem_n_pes ()
     *
     */
    int _num_pes (void)
        _WUR _DEPRECATED_BY(shmem_n_pes);

    /**
     * @brief returns the number of PEs in the program
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     int shmem_n_pes (void);
     @endcode
     *
     * @subsection f Fortran
     @code
     INTEGER I

     I = SHMEM_N_PES ()
     @endcode
     *
     * @section Effect
     *
     * None.
     *
     * @return Number of PEs in program
     *
     */
    int shmem_n_pes (void) _WUR;

    /*
     * OpenSHMEM release
     */
#define _SHMEM_MAJOR_VERSION 1
#define _SHMEM_MINOR_VERSION 2

#define _SHMEM_MAX_NAME_LEN 64

#define _SHMEM_VENDOR_STRING "UH Reference Implementation"

    /**
     * @brief determines the major.minor version numbers of this release.
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_info_get_version (int *major, int *minor);
     @endcode
     *
     * @subsection f Fortran
     @code
     INTEGER MAJ, MIN

     CALL SHMEM_INFO_GET_VERSION (MAJ, MIN)
     @endcode
     *
     * @param[out] maj set to the release's major version number
     * @param[out] min set to the release's minor version number
     *
     * @section Effect
     *
     * None.
     *
     * @return None.
     *
     */
    void shmem_info_get_version (int *major, int *minor);

    /**
     * @brief determines a vandor-supplied name for this release.
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_info_get_name (char *name);
     @endcode
     *
     * @subsection f Fortran
     @code
     CHARACTER, DIMENSION(SHMEM_MAX_NAME_LEN) :: NAME
     CALL SHMEM_INFO_GET_NAME (NAME)
     @endcode
     *
     * @param[out] name contains the vendor-supplied release name
     *
     * @section Effect
     *
     * None.
     *
     * @return None.
     *
     */
    void shmem_info_get_name (char *name);

    /*
     * I/O
     */

    /** see \ref shmem_long_put () */
    void shmem_short_put (short *dest, const short *src, size_t nelems,
                          int pe);
    /** see \ref shmem_long_put () */
    void shmem_int_put (int *dest, const int *src, size_t nelems,
                        int pe);


    void shmem_long_put (long *dest, const long *src, size_t nelems,
                         int pe);
    /** see \ref shmem_long_put () */
    void shmem_longlong_put (long long *dest, const long long *src,
                             size_t nelems, int pe);
    /** see \ref shmem_long_put () */
    void shmem_longdouble_put (long double *dest, const long double *src,
                               size_t nelems, int pe);
    /** see \ref shmem_long_put () */
    void shmem_double_put (double *dest, const double *src,
                           size_t nelems, int pe);
    /** see \ref shmem_long_put () */
    void shmem_float_put (float *dest, const float *src, size_t nelems,
                          int pe);
    /** see \ref shmem_long_put () */
    void shmem_putmem (void *dest, const void *src, size_t nelems,
                       int pe);
    /** see \ref shmem_long_put () */
    void shmem_put32 (void *dest, const void *src, size_t nelems,
                      int pe);
    /** see \ref shmem_long_put () */
    void shmem_put64 (void *dest, const void *src, size_t nelems,
                      int pe);
    /** see \ref shmem_long_put () */
    void shmem_put128 (void *dest, const void *src, size_t nelems,
                       int pe);
    /** see \ref shmem_long_get () */
    void shmem_short_get (short *dest, const short *src, size_t nelems,
                          int pe);

    /** see \ref shmem_long_get () */
    void shmem_int_get (int *dest, const int *src, size_t nelems,
                        int pe);

    void shmem_long_get (long *dest, const long *src, size_t nelems,
                         int pe);
    /** see \ref shmem_long_get () */
    void shmem_longlong_get (long long *dest, const long long *src,
                             size_t nelems, int pe);
    /** see \ref shmem_long_get () */
    void shmem_longdouble_get (long double *dest, const long double *src,
                               size_t nelems, int pe);
    /** see \ref shmem_long_get () */
    void shmem_double_get (double *dest, const double *src,
                           size_t nelems, int pe);
    /** see \ref shmem_long_get () */
    void shmem_float_get (float *dest, const float *src, size_t nelems,
                          int pe);
    /** see \ref shmem_long_get () */
    void shmem_getmem (void *dest, const void *src, size_t nelems,
                       int pe);
    /** see \ref shmem_long_get () */
    void shmem_get32 (void *dest, const void *src, size_t nelems,
                      int pe);
    /** see \ref shmem_long_get () */
    void shmem_get64 (void *dest, const void *src, size_t nelems,
                      int pe);

    /** see \ref shmem_long_get () */
    void shmem_get128 (void *dest, const void *src, size_t nelems,
                       int pe);

    /** see \ref shmem_long_p () */
    void shmem_char_p (char *addr, char value, int pe);
    /** see \ref shmem_long_p () */
    void shmem_short_p (short *addr, short value, int pe);
    /** see \ref shmem_long_p () */
    void shmem_int_p (int *addr, int value, int pe);
    void shmem_long_p (long *addr, long value, int pe);
    /** see \ref shmem_long_p () */
    void shmem_longlong_p (long long *addr, long long value, int pe);
    /** see \ref shmem_long_p () */
    void shmem_float_p (float *addr, float value, int pe);
    /** see \ref shmem_long_p () */
    void shmem_double_p (double *addr, double value, int pe);
    /** see \ref shmem_long_p () */
    void shmem_longdouble_p (long double *addr, long double value,
                             int pe);

    /** see \ref shmem_long_g () */
    char shmem_char_g (char *addr, int pe) _WUR;
    /** see \ref shmem_long_g () */
    short shmem_short_g (short *addr, int pe) _WUR;
    /** see \ref shmem_long_g () */
    int shmem_int_g (int *addr, int pe) _WUR;

    long shmem_long_g (long *addr, int pe) _WUR;
    /** see \ref shmem_long_g () */
    long long shmem_longlong_g (long long *addr, int pe) _WUR;
    /** see \ref shmem_long_g () */
    float shmem_float_g (float *addr, int pe) _WUR;
    /** see \ref shmem_long_g () */
    double shmem_double_g (double *addr, int pe) _WUR;
    /** see \ref shmem_long_g () */
    long double shmem_longdouble_g (long double *addr, int pe) _WUR;


    /*
     * strided I/O
     */

    /** see \ref shmem_long_iput () */
    void shmem_double_iput (double *target, const double *source,
                            ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                            int pe);
    /** see \ref shmem_long_iput () */
    void shmem_float_iput (float *target, const float *source,
                           ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                           int pe);
    /** see \ref shmem_long_iput () */
    void shmem_int_iput (int *target, const int *source, ptrdiff_t tst,
                         ptrdiff_t sst, size_t nelems, int pe);
    /** see \ref shmem_long_iput () */
    void shmem_iput32 (void *target, const void *source, ptrdiff_t tst,
                       ptrdiff_t sst, size_t nelems, int pe);
    /** see \ref shmem_long_iput () */
    void shmem_iput64 (void *target, const void *source, ptrdiff_t tst,
                       ptrdiff_t sst, size_t nelems, int pe);
    /** see \ref shmem_long_iput () */
    void shmem_iput128 (void *target, const void *source, ptrdiff_t tst,
                        ptrdiff_t sst, size_t nelems, int pe);

    void shmem_long_iput (long *target, const long *source,
                          ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                          int pe);
    /** see \ref shmem_long_iput () */
    void shmem_longdouble_iput (long double *target,
                                const long double *source, ptrdiff_t tst,
                                ptrdiff_t sst, size_t nelems, int pe);
    /** see \ref shmem_long_iput () */
    void shmem_longlong_iput (long long *target, const long long *source,
                              ptrdiff_t tst, ptrdiff_t sst,
                              size_t nelems, int pe);
    /** see \ref shmem_long_iput () */
    void shmem_short_iput (short *target, const short *source,
                           ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                           int pe);


    /** see \ref shmem_long_iget () */
    void shmem_double_iget (double *target, const double *source,
                            ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                            int pe);
    /** see \ref shmem_long_iget () */
    void shmem_float_iget (float *target, const float *source,
                           ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                           int pe);
    /** see \ref shmem_long_iget () */
    void shmem_int_iget (int *target, const int *source, ptrdiff_t tst,
                         ptrdiff_t sst, size_t nelems, int pe);
    /** see \ref shmem_long_iget () */
    void shmem_iget32 (void *target, const void *source, ptrdiff_t tst,
                       ptrdiff_t sst, size_t nelems, int pe);
    /** see \ref shmem_long_iget () */
    void shmem_iget64 (void *target, const void *source, ptrdiff_t tst,
                       ptrdiff_t sst, size_t nelems, int pe);
    /** see \ref shmem_long_iget () */
    void shmem_iget128 (void *target, const void *source, ptrdiff_t tst,
                        ptrdiff_t sst, size_t nelems, int pe);

    void shmem_long_iget (long *target, const long *source,
                          ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                          int pe);
    /** see \ref shmem_long_iget () */
    void shmem_longdouble_iget (long double *target,
                                const long double *source, ptrdiff_t tst,
                                ptrdiff_t sst, size_t nelems, int pe);
    /** see \ref shmem_long_iget () */
    void shmem_longlong_iget (long long *target, const long long *source,
                              ptrdiff_t tst, ptrdiff_t sst,
                              size_t nelems, int pe);
    /** see \ref shmem_long_iget () */
    void shmem_short_iget (short *target, const short *source,
                           ptrdiff_t tst, ptrdiff_t sst, size_t nelems,
                           int pe);

    /*
     * barriers
     */

    /**
     * @brief causes all PEs to synchronize
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_barrier_all (void);
     @endcode
     *
     * @subsection f Fortran
     @code
     CALL SHMEM_BARRIER_ALL ()
     @endcode
     *
     * @section Effect
     *
     * All PEs synchronize: no PE can leave the global barrier until all
     * have arrived.  Communication is also flushed before return.
     *
     * @return None.
     *
     */
    void shmem_barrier_all (void);

    /**
     * @brief causes an active set of PEs to synchronize
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_barrier (int PE_start, int logPE_stride, int PE_size,
     long *pSync);
     @endcode
     *
     * @subsection f Fortran
     @code
     INTEGER PE_start, logPE_stride, PE_size
     INTEGER (*) pSync   <-----

     CALL SHMEM_BARRIER (PE_start, logPE_stride, PE_size, pSync)
     @endcode
     *
     *
     * @param[in] PE_start first PE of the active set
     * @param[in] logPE_stride log2 of stride between PEs
     * @param[in] PE_size number of PEs in the active set
     * @param[in, out] pSync symmetric work array
     *
     * @section Effect
     *
     * PEs in the active set defined by (PE_start, logPE_stride,
     * PE_size) synchronize: no PE from this active set can leave the
     * global barrier until all have arrived.  Communication is also
     * flushed before return.  PEs not in the active set do not call
     * shmem_barrier ().  pSync must be initialized everywhere before
     * use, and, if modified, must be reset to its state before the
     * call.
     *
     * @return None.
     *
     */
    void shmem_barrier (int PE_start, int logPE_stride, int PE_size,
                        long *pSync);

    /**
     * @brief outbound communication completes before any subsequent
     * communication is sent.
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_fence (void);
     @endcode
     *
     * @subsection f Fortran
     @code
     CALL SHMEM_FENCE ()
     @endcode
     *
     * @section Effect
     *
     * BLAH
     *
     */
    void shmem_fence (void);

    /**
     * @brief causes outbound communication to complete before
     * subsequent puts are sent.
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_quiet (void);
     @endcode
     *
     * @subsection f Fortran
     @code
     CALL SHMEM_QUIET ()
     @endcode
     *
     * @section Effect
     *
     * BLAH
     *
     * @return None.
     *
     */
    void shmem_quiet (void);

    /*
     * accessibility
     */

    /**
     * @brief checks whether the caller PE can communicate with the named PE
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     int shmem_pe_accessible (int pe);
     @endcode
     *
     * @subsection f Fortran
     @code
     INTEGER PE, RET

     RET = SHMEM_PE_ACCESSIBLE (PE)
     @endcode
     *
     * @section Effect
     *
     * None
     *
     * @return non-zero if "pe" can be communicated with.  0 if not.
     *
     */
    int shmem_pe_accessible (int pe) _WUR;

    /**
     * @brief checks whether the caller PE can communicate with a memory
     * address on the named PE
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     int shmem_addr_accessible (void *addr, int pe);
     @endcode
     *
     * @subsection f Fortran
     @code
     INTEGER PE, RET
     ADDR = address    <----------------

     RET = SHMEM_ADDR_ACCESSIBLE (ADDR, PE)
     @endcode
     *
     * @param addr address to check
     * @param pe PE to check
     *
     * @section Effect
     *
     * None
     *
     * @return non-zero if address "addr" can be used for communication
     * on PE "pe".  0 if not.
     *
     */
    int shmem_addr_accessible (void *addr, int pe) _WUR;

    /**
     * @brief checks whether an address on a target PE can be accessed
     * with a simple load/store operation.
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void *shmem_ptr (void *addr, int pe);
     @endcode
     *
     * @subsection f Fortran
     @code
     INTEGER PE
     ADDR RET = address <----------
     ADDR = address    <----------------

     RET = SHMEM_PTR (ADDR, PE)
     @endcode
     *
     * @section Effect
     *
     * None
     *
     * @return a pointer to a memory location corresponding to the
     * address on the target PE if that address can be accessed with
     * load/store operations by the calling PE.  NULL if not.
     *
     */
    void *shmem_ptr (void *target, int pe) _WUR;

    /*
     * symmetric memory management
     */

    /* lower numbers match Fortran return values */

#define _SHMEM_MALLOC_OK                   (0L)
#define _SHMEM_MALLOC_BAD_SIZE             (-1L)
#define _SHMEM_MALLOC_FAIL                 (-2L)
#define _SHMEM_MALLOC_NOT_IN_SYMM_HEAP     (-3L)
#define _SHMEM_MALLOC_ALREADY_FREE         (-4L)
#define _SHMEM_MALLOC_NOT_ALIGNED          (-5L)

#define _SHMEM_MALLOC_MEMALIGN_FAILED      (-11L)
#define _SHMEM_MALLOC_REALLOC_FAILED       (-12L)
#define _SHMEM_MALLOC_SYMMSIZE_FAILED      (-10L)

#if 0

    /**
     * old names not used any more in 1.2
     */

#define SHMEM_MALLOC_OK                   _SHMEM_MALLOC_OK
#define SHMEM_MALLOC_BAD_SIZE             _SHMEM_MALLOC_BAD_SIZE
#define SHMEM_MALLOC_FAIL                 _SHMEM_MALLOC_FAIL
#define SHMEM_MALLOC_NOT_IN_SYMM_HEAP     _SHMEM_MALLOC_NOT_IN_SYMM_HEAP
#define SHMEM_MALLOC_ALREADY_FREE         _SHMEM_MALLOC_ALREADY_FREE
#define SHMEM_MALLOC_NOT_ALIGNED          _SHMEM_MALLOC_NOT_ALIGNED

#define SHMEM_MALLOC_MEMALIGN_FAILED      _SHMEM_MALLOC_MEMALIGN_FAILED
#define SHMEM_MALLOC_REALLOC_FAILED       _SHMEM_MALLOC_REALLOC_FAILED
#define SHMEM_MALLOC_SYMMSIZE_FAILED      _SHMEM_MALLOC_SYMMSIZE_FAILED
#endif

#if 0
    long malloc_error;
#endif  /* not present in SGI version */

    /* deprecated calls from 1.2 ++ */

    /**
     * @brief dynamically allocates symmetric memory
     *
     * @deprecated by \ref shmem_malloc ()
     *
     */
    void *shmalloc (size_t size)
        _WUR _DEPRECATED_BY(shmem_malloc);

    /**
     * @brief dynamically allocates symmetric memory
     *
     * @deprecated by \ref shmem_free ()
     *
     */
    void shfree (void *ptr)
        _DEPRECATED_BY(shmem_free);

    /**
     * @brief dynamically allocates symmetric memory
     *
     * @deprecated by \ref shmem_realloc ()
     *
     */
    void *shrealloc (void *ptr, size_t size)
        _WUR _DEPRECATED_BY(shmem_realloc);

    /**
     * @brief dynamically allocates symmetric memory
     *
     * @deprecated by \ref shmem_align ()
     *
     */
    void *shmemalign (size_t alignment, size_t size)
        _WUR _DEPRECATED_BY(shmem_align);

    /**
     * @brief dynamically allocates symmetric memory
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void *shmalloc (size_t size);
     @endcode
     *
     * @param size numer of bytes requested
     *
     * @section Effect
     *
     * Allocates "size" bytes of memory from the PE's symmetric heap.
     *
     * @return a pointer to the requested memory location, or NULL if
     * the requested memory is not available.
     *
     */
    void *shmem_malloc (size_t size) _WUR;

    /**
     * @brief dynamically allocates symmetric memory
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shfree (void *ptr);
     @endcode
     *
     * @param ptr symmetric memory pointer
     *
     * @section Effect
     *
     * Frees a previous symmetric allocation.
     *
     */
    void shmem_free (void *ptr);

    /**
     * @brief dynamically allocates symmetric memory
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void *shrealloc (void *ptr, size_t size);
     @endcode
     *
     * @param ptr symmetric memory pointer
     * @param size number of bytes
     *
     * @section Effect
     *
     * Resizes a previous symmetric memory allocation starting at "ptr"
     * to "size" bytes.
     *
     * @return a pointer to the resized area, or NULL if this is not
     * possible.
     *
     */
    void *shmem_realloc (void *ptr, size_t size) _WUR;

    /**
     * @brief dynamically allocates symmetric memory
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void *shrealloc (void *ptr, size_t size);
     @endcode
     *
     * @param ptr symmetric memory pointer
     * @param size number of bytes
     *
     * @section Effect
     *
     * Resizes a previous symmetric memory allocation starting at "ptr"
     * to "size" bytes.
     *
     * @return a pointer to the resized area, or NULL if this is not
     * possible.
     *
     */
    void *shmem_align (size_t alignment, size_t size) _WUR;

    /*
     * wait operations
     */

    /*
     * values aren't important
     */

    enum shmem_cmp_constants
        {
            _SHMEM_CMP_EQ = 0,
            _SHMEM_CMP_NE,
            _SHMEM_CMP_GT,
            _SHMEM_CMP_LE,
            _SHMEM_CMP_LT,
            _SHMEM_CMP_GE,
#if 0
            SHMEM_CMP_EQ = 0,
            SHMEM_CMP_NE,
            SHMEM_CMP_GT,
            SHMEM_CMP_LE,
            SHMEM_CMP_LT,
            SHMEM_CMP_GE,
#endif
        };

    /**
     * @brief wait for symmetric variable to change value
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_long_wait_until (long *ivar, int cmp, long cmp_value);
     @endcode
     *
     * @subsection f Fortran
     @code
     ...
     @endcode
     *
     * @section Effect
     *
     * var updated by another PE, wait for that to happen blah...
     *
     * @return None.
     *
     */
    void shmem_long_wait_until (long *ivar, int cmp, long cmp_value);

    /** see \ref shmem_long_wait_until () */
    void shmem_short_wait_until (short *ivar, int cmp, short cmp_value);
    /** see \ref shmem_long_wait_until () */
    void shmem_int_wait_until (int *ivar, int cmp, int cmp_value);
    /** see \ref shmem_long_wait_until () */
    void shmem_longlong_wait_until (long long *ivar, int cmp,
                                    long long cmp_value);
    /** see \ref shmem_long_wait_until () */
    void shmem_wait_until (long *ivar, int cmp, long cmp_value);

    /**
     * @brief wait for symmetric variable to change value
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_long_wait (long *ivar, long cmp_value);
     @endcode
     *
     * @subsection f Fortran
     @code
     ...
     @endcode
     *
     * @section Effect
     *
     * var updated by another PE, wait for that to happen blah...
     *
     * @return None.
     *
     */
    void shmem_long_wait (long *ivar, long cmp_value);

    /** see \ref shmem_long_wait () */
    void shmem_short_wait (short *ivar, short cmp_value);
    /** see \ref shmem_long_wait () */
    void shmem_int_wait (int *ivar, int cmp_value);
    /** see \ref shmem_long_wait () */
    void shmem_longlong_wait (long long *ivar, long long cmp_value);
    /** see \ref shmem_long_wait () */
    void shmem_wait (long *ivar, long cmp_value);

    /*
     * atomic swaps
     */

    /**
     * @brief swap value into symmetric variable, fetch back old value
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     long shmem_long_swap (long *target, long value, int pe);
     @endcode
     *
     * @subsection f Fortran
     @code
     ...
     @endcode
     *
     * @section Effect
     *
     * swaps things!
     *
     * @return None.
     *
     */
    long shmem_long_swap (long *target, long value, int pe) _WUR;

    /** see \ref shmem_long_swap () */
    int shmem_int_swap (int *target, int value, int pe) _WUR;
    /** see \ref shmem_long_swap () */
    long long shmem_longlong_swap (long long *target, long long value,
                                   int pe) _WUR;
    /** see \ref shmem_long_swap () */
    float shmem_float_swap (float *target, float value, int pe) _WUR;
    /** see \ref shmem_long_swap () */
    double shmem_double_swap (double *target, double value, int pe) _WUR;
    /** see \ref shmem_long_swap () */
    long shmem_swap (long *target, long value, int pe) _WUR;

    /**
     * @brief conditionally swap value into symmetric variable, fetch
     * back old value
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     long shmem_long_cswap (long *target, long cond, long value, int pe);
     @endcode
     *
     * @subsection f Fortran
     @code
     ...
     @endcode
     *
     * @section Effect
     *
     * swaps things!  conditionally!
     *
     * @return None.
     *
     */
    long shmem_long_cswap (long *target, long cond, long value,
                           int pe) _WUR;

    /** see \ref shmem_long_cswap () */
    int shmem_int_cswap (int *target, int cond, int value, int pe) _WUR;
    /** see \ref shmem_long_cswap () */
    long long shmem_longlong_cswap (long long *target, long long cond,
                                    long long value, int pe) _WUR;

    /*
     * atomic fetch-{add,inc} & add,inc
     */

    /**
     * @brief add value to symmetric variable, fetch back old value
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     long shmem_long_fadd (long *target, long value, int pe);
     @endcode
     *
     * @subsection f Fortran
     @code
     ...
     @endcode
     *
     * @section Effect
     *
     * atomic fetch-and-add on another PE
     *
     * @return None.
     *
     */
    long shmem_long_fadd (long *target, long value, int pe) _WUR;

    /** see \ref shmem_long_fadd () */
    int shmem_int_fadd (int *target, int value, int pe) _WUR;
    /** see \ref shmem_long_fadd () */
    long long shmem_longlong_fadd (long long *target, long long value,
                                   int pe) _WUR;

    /**
     * @brief increment symmetric variable, fetch back old value
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     long shmem_long_finc (long *target, int pe);
     @endcode
     *
     * @subsection f Fortran
     @code
     ...
     @endcode
     *
     * @section Effect
     *
     * atomic fetch-and-increment on another PE
     *
     * @return None.
     *
     */
    long shmem_long_finc (long *target, int pe) _WUR;

    /** see \ref shmem_long_finc () */
    int shmem_int_finc (int *target, int pe) _WUR;
    /** see \ref shmem_long_finc () */
    long long shmem_longlong_finc (long long *target, int pe) _WUR;

    /**
     * @brief add value to symmetric variable
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_long_add (long *target, long value, int pe);
     @endcode
     *
     * @subsection f Fortran
     @code
     ...
     @endcode
     *
     * @section Effect
     *
     * atomic add on another PE
     *
     * @return None.
     *
     */
    void shmem_long_add (long *target, long value, int pe);

    /** see \ref shmem_long_add () */
    void shmem_int_add (int *target, int value, int pe);
    /** see \ref shmem_long_add () */
    void shmem_longlong_add (long long *target, long long value, int pe);

    /**
     * @brief increment symmetric variable
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_long_inc (long *target, int pe);
     @endcode
     *
     * @subsection f Fortran
     @code
     ...
     @endcode
     *
     * @section Effect
     *
     * atomic increment on another PE
     *
     * @return None.
     *
     */
    void shmem_long_inc (long *target, int pe);

    /** see \ref shmem_long_inc () */
    void shmem_int_inc (int *target, int pe);
    /** see \ref shmem_long_inc () */
    void shmem_longlong_inc (long long *target, int pe);

    /*
     * cache flushing (deprecated)
     */

    /**
     * @brief shmem_clear_cache_inv has no effect.
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_clear_cache_inv (void);
     @endcode
     *
     * @subsection f Fortran
     @code
     CALL SHMEM_CLEAR_CACHE_INV()
     @endcode
     *
     * @section Effect
     *
     * None.
     *
     * @deprecated Included for legacy use only, these calls have no
     * effect in OpenSHMEM.
     *
     * @return None.
     *
     */
    void shmem_clear_cache_inv (void) _DEPRECATED;

    /**
     * @brief shmem_set_cache_inv has no effect.
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_set_cache_inv (void);
     @endcode
     *
     * @subsection f Fortran
     @code
     CALL SHMEM_SET_CACHE_INV()
     @endcode
     *
     * @section Effect
     *
     * None.  Included for legacy use only, these calls have no
     * effect in OpenSHMEM.
     *
     * @deprecated Included for legacy use only.
     *
     * @return None.
     *
     */
    void shmem_set_cache_inv (void) _DEPRECATED;

    /**
     * @brief shmem_clear_cache_line_inv has no effect.
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_clear_cache_line_inv (void *target);
     @endcode
     *
     * @subsection f Fortran
     @code
     CALL SHMEM_CLEAR_CACHE_LINE_INV(target)
     @endcode
     *
     * @param target is the address of the cache line.
     *
     * @section Effect
     *
     * None.
     *
     * @deprecated Included for legacy use only, these calls have no
     * effect in OpenSHMEM.
     *
     * @return None.
     *
     */
    void shmem_clear_cache_line_inv (void *target) _DEPRECATED;

    /**
     * @brief shmem_set_cache_line_inv has no effect.
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_set_cache_line_inv (void *target);
     @endcode
     *
     * @subsection f Fortran
     @code
     CALL SHMEM_SET_CACHE_LINE_INV(target)
     @endcode
     *
     * @param target is the address of the cache line.
     *
     * @section Effect
     *
     * None.
     *
     * @deprecated Included for legacy use only, these calls have no
     * effect in OpenSHMEM.
     *
     * @return None.
     *
     */
    void shmem_set_cache_line_inv (void *target) _DEPRECATED;

    /**
     * @brief shmem_udcflush has no effect.
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_udcflush (void);
     @endcode
     *
     * @subsection f Fortran
     @code
     CALL SHMEM_UDCFLUSH()
     @endcode
     *
     * @section Effect
     *
     * None.
     *
     * @deprecated Included for legacy use only, these calls have no
     * effect in OpenSHMEM.
     *
     * @return None.
     *
     */
    void shmem_udcflush (void) _DEPRECATED;

    /**
     * @brief shmem_udcflush_line has no effect.
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_udcflush_line (void *target);
     @endcode
     *
     * @subsection f Fortran
     @code
     CALL SHMEM_UDCFLUSH_LINE(target)
     @endcode
     *
     * @param target is the address of the cache line.
     *
     * @section Effect
     *
     * None.
     *
     * @deprecated Included for legacy use only, these calls have no
     * effect in OpenSHMEM.
     *
     * @return None.
     *
     */
    void shmem_udcflush_line (void *target) _DEPRECATED;

    /*
     * reductions
     */

    /*
     * cf. Fortran values are multiples of these (different types)
     */
#define SHMEM_INTERNAL_F2C_SCALE ( sizeof (long) / sizeof (int) )

#define _SHMEM_BCAST_SYNC_SIZE (128L / SHMEM_INTERNAL_F2C_SCALE)
#define _SHMEM_BARRIER_SYNC_SIZE (128L / SHMEM_INTERNAL_F2C_SCALE)
#define _SHMEM_REDUCE_SYNC_SIZE (256L / SHMEM_INTERNAL_F2C_SCALE)
#define _SHMEM_REDUCE_MIN_WRKDATA_SIZE (128L / SHMEM_INTERNAL_F2C_SCALE)

#if 0
#define SHMEM_BCAST_SYNC_SIZE _SHMEM_BCAST_SYNC_SIZE
#define SHMEM_BARRIER_SYNC_SIZE _SHMEM_BARRIER_SYNC_SIZE
#define SHMEM_REDUCE_SYNC_SIZE _SHMEM_REDUCE_SYNC_SIZE
#define SHMEM_REDUCE_MIN_WRKDATA_SIZE _SHMEM_REDUCE_MIN_WRKDATA_SIZE
#endif

    /*
     * Initialize sync arrays to this
     */
#define _SHMEM_SYNC_VALUE (-1L)

#if 0
#define SHMEM_SYNC_VALUE _SHMEM_SYNC_VALUE
#endif

    void shmem_long_sum_to_all (long *target, long *source, int nreduce,
                                int PE_start, int logPE_stride,
                                int PE_size, long *pWrk, long *pSync);

    /** see \ref shmem_long_sum_to_all () */
    void shmem_complexd_sum_to_all (COMPLEXIFY (double) * target,
                                    COMPLEXIFY (double) * source,
                                    int nreduce,
                                    int PE_start, int logPE_stride,
                                    int PE_size,
                                    COMPLEXIFY (double) * pWrk,
                                    long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_complexf_sum_to_all (COMPLEXIFY (float) * target,
                                    COMPLEXIFY (float) * source,
                                    int nreduce, int PE_start,
                                    int logPE_stride, int PE_size,
                                    COMPLEXIFY (float) * pWrk,
                                    long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_double_sum_to_all (double *target, double *source,
                                  int nreduce, int PE_start,
                                  int logPE_stride, int PE_size,
                                  double *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_float_sum_to_all (float *target, float *source,
                                 int nreduce, int PE_start,
                                 int logPE_stride, int PE_size,
                                 float *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_int_sum_to_all (int *target, int *source, int nreduce,
                               int PE_start, int logPE_stride,
                               int PE_size, int *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_longdouble_sum_to_all (long double *target,
                                      long double *source, int nreduce,
                                      int PE_start, int logPE_stride,
                                      int PE_size, long double *pWrk,
                                      long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_longlong_sum_to_all (long long *target, long long *source,
                                    int nreduce, int PE_start,
                                    int logPE_stride, int PE_size,
                                    long long *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_short_sum_to_all (short *target, short *source,
                                 int nreduce, int PE_start,
                                 int logPE_stride, int PE_size,
                                 short *pWrk, long *pSync);

    /** see \ref shmem_long_sum_to_all () */
    void shmem_complexd_prod_to_all (COMPLEXIFY (double) * target,
                                     COMPLEXIFY (double) * source,
                                     int nreduce,
                                     int PE_start, int logPE_stride,
                                     int PE_size,
                                     COMPLEXIFY (double) * pWrk,
                                     long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_complexf_prod_to_all (COMPLEXIFY (float) * target,
                                     COMPLEXIFY (float) * source,
                                     int nreduce, int PE_start,
                                     int logPE_stride, int PE_size,
                                     COMPLEXIFY (float) * pWrk,
                                     long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_double_prod_to_all (double *target, double *source,
                                   int nreduce, int PE_start,
                                   int logPE_stride, int PE_size,
                                   double *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_float_prod_to_all (float *target, float *source,
                                  int nreduce, int PE_start,
                                  int logPE_stride, int PE_size,
                                  float *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_int_prod_to_all (int *target, int *source, int nreduce,
                                int PE_start, int logPE_stride,
                                int PE_size, int *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_long_prod_to_all (long *target, long *source, int nreduce,
                                 int PE_start, int logPE_stride,
                                 int PE_size, long *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_longdouble_prod_to_all (long double *target,
                                       long double *source, int nreduce,
                                       int PE_start, int logPE_stride,
                                       int PE_size, long double *pWrk,
                                       long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_longlong_prod_to_all (long long *target,
                                     long long *source, int nreduce,
                                     int PE_start, int logPE_stride,
                                     int PE_size, long long *pWrk,
                                     long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_short_prod_to_all (short *target, short *source,
                                  int nreduce, int PE_start,
                                  int logPE_stride, int PE_size,
                                  short *pWrk, long *pSync);

    /** see \ref shmem_long_sum_to_all () */
    void shmem_int_and_to_all (int *target,
                               int *source,
                               int nreduce,
                               int PE_start, int logPE_stride,
                               int PE_size, int *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_long_and_to_all (long *target, long *source, int nreduce,
                                int PE_start, int logPE_stride,
                                int PE_size, long *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_longlong_and_to_all (long long *target, long long *source,
                                    int nreduce, int PE_start,
                                    int logPE_stride, int PE_size,
                                    long long *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_short_and_to_all (short *target, short *source,
                                 int nreduce, int PE_start,
                                 int logPE_stride, int PE_size,
                                 short *pWrk, long *pSync);

    /** see \ref shmem_long_sum_to_all () */
    void shmem_int_or_to_all (int *target,
                              int *source,
                              int nreduce,
                              int PE_start, int logPE_stride,
                              int PE_size, int *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_long_or_to_all (long *target, long *source, int nreduce,
                               int PE_start, int logPE_stride,
                               int PE_size, long *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_longlong_or_to_all (long long *target, long long *source,
                                   int nreduce, int PE_start,
                                   int logPE_stride, int PE_size,
                                   long long *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_short_or_to_all (short *target, short *source,
                                int nreduce, int PE_start,
                                int logPE_stride, int PE_size,
                                short *pWrk, long *pSync);

    /** see \ref shmem_long_sum_to_all () */
    void shmem_int_xor_to_all (int *target,
                               int *source,
                               int nreduce,
                               int PE_start, int logPE_stride,
                               int PE_size, int *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_long_xor_to_all (long *target, long *source, int nreduce,
                                int PE_start, int logPE_stride,
                                int PE_size, long *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_longlong_xor_to_all (long long *target, long long *source,
                                    int nreduce, int PE_start,
                                    int logPE_stride, int PE_size,
                                    long long *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_short_xor_to_all (short *target, short *source,
                                 int nreduce, int PE_start,
                                 int logPE_stride, int PE_size,
                                 short *pWrk, long *pSync);

    /** see \ref shmem_long_sum_to_all () */
    void shmem_int_max_to_all (int *target,
                               int *source,
                               int nreduce,
                               int PE_start, int logPE_stride,
                               int PE_size, int *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_long_max_to_all (long *target, long *source, int nreduce,
                                int PE_start, int logPE_stride,
                                int PE_size, long *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_longlong_max_to_all (long long *target, long long *source,
                                    int nreduce, int PE_start,
                                    int logPE_stride, int PE_size,
                                    long long *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_short_max_to_all (short *target, short *source,
                                 int nreduce, int PE_start,
                                 int logPE_stride, int PE_size,
                                 short *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_longdouble_max_to_all (long double *target,
                                      long double *source, int nreduce,
                                      int PE_start, int logPE_stride,
                                      int PE_size, long double *pWrk,
                                      long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_float_max_to_all (float *target, float *source,
                                 int nreduce, int PE_start,
                                 int logPE_stride, int PE_size,
                                 float *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_double_max_to_all (double *target, double *source,
                                  int nreduce, int PE_start,
                                  int logPE_stride, int PE_size,
                                  double *pWrk, long *pSync);

    /** see \ref shmem_long_sum_to_all () */
    void shmem_int_min_to_all (int *target,
                               int *source,
                               int nreduce,
                               int PE_start, int logPE_stride,
                               int PE_size, int *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_long_min_to_all (long *target, long *source, int nreduce,
                                int PE_start, int logPE_stride,
                                int PE_size, long *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_longlong_min_to_all (long long *target, long long *source,
                                    int nreduce, int PE_start,
                                    int logPE_stride, int PE_size,
                                    long long *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_short_min_to_all (short *target, short *source,
                                 int nreduce, int PE_start,
                                 int logPE_stride, int PE_size,
                                 short *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_longdouble_min_to_all (long double *target,
                                      long double *source, int nreduce,
                                      int PE_start, int logPE_stride,
                                      int PE_size, long double *pWrk,
                                      long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_float_min_to_all (float *target, float *source,
                                 int nreduce, int PE_start,
                                 int logPE_stride, int PE_size,
                                 float *pWrk, long *pSync);
    /** see \ref shmem_long_sum_to_all () */
    void shmem_double_min_to_all (double *target, double *source,
                                  int nreduce, int PE_start,
                                  int logPE_stride, int PE_size,
                                  double *pWrk, long *pSync);

    /*
     * broadcasts
     */

    void shmem_broadcast64 (void *target, const void *source,
                            size_t nelems, int PE_root, int PE_start,
                            int logPE_stride, int PE_size, long *pSync);

    /** see \ref shmem_broadcast64 () */
    void shmem_broadcast32 (void *target, const void *source,
                            size_t nelems, int PE_root, int PE_start,
                            int logPE_stride, int PE_size, long *pSync);

    /*
     * collects
     */

#define _SHMEM_COLLECT_SYNC_SIZE (128L / SHMEM_INTERNAL_F2C_SCALE)
#if 0
#define SHMEM_COLLECT_SYNC_SIZE _SHMEM_COLLECT_SYNC_SIZE
#endif

    void shmem_fcollect64 (void *target, const void *source,
                           size_t nelems, int PE_start, int logPE_stride,
                           int PE_size, long *pSync);

    /** see \ref shmem_fcollect64 () */
    void shmem_fcollect32 (void *target, const void *source,
                           size_t nelems, int PE_start, int logPE_stride,
                           int PE_size, long *pSync);

    void shmem_collect64 (void *target, const void *source,
                          size_t nelems, int PE_start, int logPE_stride,
                          int PE_size, long *pSync);
    /** see \ref shmem_collect64 () */
    void shmem_collect32 (void *target, const void *source,
                          size_t nelems, int PE_start, int logPE_stride,
                          int PE_size, long *pSync);

    /*
     * locks/critical section
     */

    /**
     * @brief claims a distributed lock
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_set_lock (long *lock);
     @endcode
     *
     * @subsection f Fortran
     @code
     INTEGER LOCK

     CALL SHMEM_SET_LOCK (LOCK)
     @endcode
     *
     * @param[in, out] lock a symmetric variable
     *
     * @section Effect
     *
     * The calling PE claims a lock on the symmetric variable.  Blocks
     * until lock acquired.
     *
     * @return None.
     *
     */
    void shmem_set_lock (long *lock);

    /**
     * @brief releases a distributed lock
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_clear_lock (long *lock);
     @endcode
     *
     * @subsection f Fortran
     @code
     INTEGER LOCK

     CALL SHMEM_CLEAR_LOCK (LOCK)
     @endcode
     *
     * @param[in, out] lock a symmetric variable
     *
     * @section Effect
     *
     * The calling PE releases a lock on the symmetric variable.
     *
     * @return None.
     *
     */
    void shmem_clear_lock (long *lock);

    /**
     * @brief tests a distributed lock
     *
     * @section Synopsis
     *
     * @subsection c C/C++
     @code
     void shmem_test_lock (long *lock);
     @endcode
     *
     * @subsection f Fortran
     @code
     INTEGER LOCK

     CALL SHMEM_TEST_LOCK (LOCK)
     @endcode
     *
     * @param[in, out] lock a symmetric variable
     *
     * @section Effect
     *
     * The calling PE checks to see if lock can be acquired.  If yes,
     * the lock is claimed, otherwise the lock is not claimed and the
     * call returns immediately.  until lock acquired.
     *
     * @return non-zero if lock acquired, 0 if not.
     *
     */
    int shmem_test_lock (long *lock) _WUR;

    /*
     * --end--
     */

#ifdef __cplusplus
}
#endif  /* __cplusplus */

/*
 * tidy-up complex "I" macro detection.
 *
 */
#ifdef  shmemi_h_I_already_defined__
# undef  shmemi_h_I_already_defined__
#else
# undef I
#endif /* shmemi_h_I_already_defined__ */

#endif  /* _SHMEM_H */
