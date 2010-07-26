.PHONY: all default clean

all default:	dlmalloc.c
	gcc -fPIC -c -DONLY_MSPACES=1 -DUSE_LOCKS=1 -g -O3 $^

clean:
	rm -f dlmalloc.o
