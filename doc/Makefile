include ../make.include

SHMEM_MANPAGES_1 = $(wildcard *.1)
SHMEM_MANPAGES_3 = $(wildcard *.3)

.PHONY: all default
.PHONY: install install-man
.PHONY: clean

all default:

install:	install-man1	install-man3

install-man1:	all
	mkdir -p $(PREFIX)/man/man1
	install -c  $(SHMEM_MANPAGES_1) $(PREFIX)/man/man1

install-man3:	all
	mkdir -p $(PREFIX)/man/man3
	# install -c  $(SHMEM_MANPAGES_3) $(PREFIX)/man/man3

clean:
