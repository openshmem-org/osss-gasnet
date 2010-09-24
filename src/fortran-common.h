#ifndef FORTRAN_COMMON_H
#define FORTRAN_COMMON_H 1

#ifdef FORTRAN_SINGLE_UNDERSCORE

#define FORTRANIFY(sym)    sym##_

#else /* ! FORTRAN_SINGLE_UNDERSCORE */

#define FORTRANIFY(sym)    sym##__

#endif /* FORTRAN_SINGLE_UNDERSCORE */

#define FORTRANIFY_VOID_VOID(F) \
  void FORTRANIFY(F) (void) { F(); }

#endif /* FORTRAN_COMMON_H */
