SHELL = /bin/sh

help: 
	@echo
	@echo "Please use one of the following targets:"
	@echo
	@echo "    * For Examples, C feature tests, Fortran feature tests and Microbenchmarks : compile, run, clean"
	@echo "    * For Examples : make examples, make examples-run"
	@echo "    * For C feature tests : make C_feature_tests, make C_feature_tests-run "
	@echo "    * For  Fortran feature tests : make F_feature_tests, make F_feature_tests-run"
	@echo "    * For  Microbenchmarks : make micro_bench, make micro_bench-run"
	@echo
	@echo "    clean: removes object files"
	@echo

.PHONY : examples

compile: C_feature_tests F_feature_tests micro_bench examples 

run: examples-run C_feature_tests-run F_feature_tests-run micro_bench-run

C_feature_tests: 
	cd feature_tests/C; make all

examples:
	cd examples; make all

examples-run:
	cd examples; make run  


C_feature_tests-run:
	cd feature_tests/C; make run

F_feature_tests: 
	cd feature_tests/Fortran; make all

F_feature_tests-run:
	cd feature_tests/Fortran; make run

micro_bench:
	cd performance_tests/micro_benchmarks; make 

micro_bench-run:
	cd performance_tests/micro_benchmarks; make run 

clean:
	cd examples; make clean
	cd feature_tests/C; make clean
	cd feature_tests/Fortran; make clean
	cd performance_tests/micro_benchmarks; make clean
	rm -f *~ */*~ *.x *.o

