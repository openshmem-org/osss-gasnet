.PHONY: all default clean

all default:	dlmalloc.o
	gcc -fPIC -c -DONLY_MSPACES=1 -DUSE_LOCKS=1 -g dlmalloc.c 

clean:
	rm -f dlmalloc.o
