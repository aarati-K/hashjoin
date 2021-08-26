
.PHONY: all clean distclean

include system.inc

CXX=g++
# CPPFLAGS=-Idist/include/
#CPPFLAGS+=-DPREFETCH
#CPPFLAGS+=-DOUTPUT_AGGREGATE
#CPPFLAGS+=-DOUTPUT_WRITE_NT
#CPPFLAGS+=-DOUTPUT_WRITE_NORMAL
CPPFLAGS=-DOUTPUT_ASSEMBLE
#CPPFLAGS+=-DDEBUG #-DDEBUG2
CXXFLAGS=$(SYSFLAGS)
#CXXFLAGS+=-g -O0 #-Wall
CXXFLAGS+=-O3
# LDFLAGS=-Ldist/lib/
# LDLIBS=-lconfig++ -lpthread

# ifeq ($(HOSTTYPE),sparc)
# LDLIBS+=-lcpc
# CXXFLAGS+=-mcpu=ultrasparc
# endif

all: canonical

FILES = table.o hashjoin.o hashjoinvip.o metrics.o


clean:
	rm -f *.o
	rm -f *.out
	rm -f canonical

canonical: $(FILES) canonical.cpp

tpch: $(FILES) tpch.cpp

