.PHONY: all clean

CXX=g++
CXXFLAGS=-O3
CXXFLAGS+=-march=native

all: canonical

FILES = table.o hashjoin.o hashjoinvip.o metrics.o

clean:
	rm -f *.o
	rm -f *.out
	rm -f canonical

canonical: $(FILES) canonical.cpp

