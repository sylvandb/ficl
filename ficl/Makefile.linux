OBJECTS= dict.o ficl.o math64.o softcore.o stack.o sysdep.o vm.o words.o
HEADERS= ficl.h math64.h sysdep.h
#
# Flags for shared library
SHFLAGS = -fPIC
CFLAGS= -O -c $(SHFLAGS)
CC=gcc
LIB = ar cr
RANLIB = ranlib

MAJOR = 2
MINOR = 0.3

lib: libficl.so.$(MAJOR).$(MINOR)

# static library build
libficl.a: $(OBJECTS)
	$(LIB) libficl.a $(OBJECTS)
	$(RANLIB) libficl.a

# shared library build
libficl.so.$(MAJOR).$(MINOR): $(OBJECTS)
	$(CC) -shared -Wl,-soname,libficl.so.$(MAJOR).$(MINOR) \
	-o libficl.so.$(MAJOR).$(MINOR) $(OBJECTS)
	ln -s libficl.so.$(MAJOR).$(MINOR) libficl.so

testmain: testmain.o ficl.h sysdep.h libficl.so.$(MAJOR).$(MINOR)
	$(CC) testmain.o -o testmain -L. -lficl -lm
	ln -s libficl.so.$(MAJOR).$(MINOR) libficl.so.$(MAJOR)

#
#       generic object code
#
.SUFFIXES: .cxx .cc .c .o

.c.o:
	$(CC) $(CFLAGS) -c $*.c

.cxx.o:
	$(CPP) $(CPFLAGS) -c $*.cxx

.cc.o:
	$(CPP) $(CPFLAGS) -c $*.cc
#
#       generic cleanup code
#
clean:
	rm -f *.o *.a testmain libficl.so*
