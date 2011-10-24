BEGIN { SMAKE = "make" } {
  if ($1 !~ /^#/ &&  NF > 2) {
    printf "cd `echo %s|tr '[a-z]' '[A-Z]'`; %s clean;", $1, SMAKE;
    if ( NF > 3 ) {
      printf "%s CLASS=%s NPROCS=%s SUBTYPE=%s; cd ..\n", SMAKE, $2, $3, $4;
    }
    else {
      printf "%s CLASS=%s NPROCS=%s; cd ..\n", SMAKE, $2, $3;
    }
  }
}
