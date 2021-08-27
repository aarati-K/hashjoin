
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

all: canonical config_1 config_2 config_3 config_4

FILES = table.o hashjoin.o hashjoinvip.o metrics.o


clean:
	rm -f *.o
	rm -f *.out
	rm -f canonical

canonical: $(FILES) canonical.cpp

config_1: $(FILES) config_1.cpp

config_2: $(FILES) config_2.cpp

config_3: $(FILES) config_3.cpp

config_4: $(FILES) config_4.cpp

tpch: $(FILES) tpch.cpp
