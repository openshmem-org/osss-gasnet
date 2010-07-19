SHMEM_FC = oshfort

TESTS = hello.f90 multi-init.f90

EXES = $(TESTS:.f90=.x)

.PHONY: clean all default

all default:	$(EXES)

.SUFFIXES: .f90 .x

%.x:	%.f90
	$(SHMEM_FC) -o $@ $^

clean:
	rm -f $(EXES)
