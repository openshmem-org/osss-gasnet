prefix           = /home/tonyc/opt/OpenSHMEM

CC               = gcc
CFLAGS           = -ggdb -O3
CPPFLAGS         = 
LD               = gcc
LDFLAGS          = 

ifeq "TRUE" "TRUE"
CPPFLAGS        += -DDEBUG
endif

# CPPFLAGS        += -I.
CPPFLAGS        += -I..
CPPFLAGS        += -DMODULES_DIR="\"$(prefix)/lib/modules\""

CFLAGS          += -fPIC

.PHONY: all default clean

SOURCES  = $(wildcard *.c)
OBJECTS  = $(SOURCES:.c=.o)

all default:	$(OBJECTS)

clean:
	rm -f $(OBJECTS)
