.PHONY: all default clean

all default clean:
	for d in * ; do [ -d "$$d" ] && $(MAKE) -C "$$d" $@ ; done
