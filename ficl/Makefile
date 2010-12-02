SRCDIR=	.
VPATH=	$(SRCDIR):$(SRCDIR)/softcore
# core:
OBJECTS= dictionary.o system.o fileaccess.o float.o double.o prefix.o search.o softcore.o stack.o tools.o vm.o primitives.o bit.o lzuncompress.o utility.o hash.o callback.o word.o extras.o $(OBJECTS.$(platform))
HEADERS= ficl.h $(HEADERS.$(platform))
# platform dependent source:
OBJECTS.unix=	unix.o
HEADERS.unix=	ficlplatform/unix.h
OBJECTS.ansi=	ansi.o
HEADERS.ansi=	ficlplatform/ansi.h
# soft core:
SOURCES = softcore.fr ifbrack.fr prefix.fr ficl.fr jhlocal.fr marker.fr oo.fr classes.fr string.fr ficllocal.fr fileaccess.fr
#
# Flags for shared library
SHFLAGS = -fPIC
CFLAGS= $(CFLAGS.$(platform)) -O $(SHFLAGS) -Wall
CPPFLAGS= $(CPPFLAGS.$(platform)) -I.
# Platform dependent flags:
CFLAGS.unix=	# empty
CPPFLAGS.unix=	# empty
CFLAGS.ansi=	-ansi
CPPFLAGS.ansi=	-ansi -DFICL_ANSI
# Default platform is "unix"
platform=	unix
#
CC = cc
LIB = ar cr
RANLIB = ranlib

MAJOR = 4
MINOR = 1.0

.PHONY: all test clean
.PHONY: lib

all: ficl

test:
	wd="$$(pwd)"; cd test && $${wd}/ficl ficltest.fr < /dev/null

ficl: main.o $(HEADERS) libficl.a
	$(CC) $(CFLAGS) $(LDFLAGS) main.o -o ficl -L. -lficl -lm

lib: libficl.so.$(MAJOR).$(MINOR)

# static library build
libficl.a: $(OBJECTS)
	$(LIB) libficl.a $(OBJECTS)
	$(RANLIB) libficl.a

# shared library build
libficl.so.$(MAJOR).$(MINOR): $(OBJECTS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,libficl.so.$(MAJOR).$(MINOR) \
	-o libficl.so.$(MAJOR).$(MINOR) $(OBJECTS)
	ln -sf libficl.so.$(MAJOR).$(MINOR) libficl.so

main: main.o ficl.h sysdep.h libficl.so.$(MAJOR).$(MINOR)
	$(CC) $(CFLAGS) $(LDFLAGS) main.o -o main -L. -lficl -lm
	ln -sf libficl.so.$(MAJOR).$(MINOR) libficl.so.$(MAJOR)

# depend explicitly to help finding source files in another subdirectory,
# and repeat commands since gmake doesn't understand otherwise
ansi.o: ficlplatform/ansi.c $(HEADERS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
unix.o: ficlplatform/unix.c $(HEADERS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

# generate softcore source
softcore.c: $(SOURCES)
	(cd softcore && exec $(MAKE) softcore.c) && cp softcore/softcore.c .

#
#       generic object code
#
.SUFFIXES: .cxx .cc .c .o

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

.cxx.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

.cc.o:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
#
#       generic cleanup code
#
clean:
	rm -f *.o *.a *.core libficl.* ficl
	rm -f softcore.c softcore/softcore.c softcore/makesoftcore
