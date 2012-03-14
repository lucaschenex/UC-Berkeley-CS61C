HOME = /home/ff/cs61c
CC = gcc-4.5 -std=gnu99 
GOTO = $(HOME)/bin/GotoBLAS2_Linux
GOTOLIB = $(GOTO)/libgoto2_nehalemp-r1.13.a

INCLUDES = -I$(GOTO)
OMP = -fopenmp
LIBS = -lpthread  
# a pretty good flag selection for this machine...
CFLAGS = -fopenmp -O2 -pipe -fno-omit-frame-pointer -march=native

all:	bench-naive bench-small bench-all bench-openmp 

# triple nested loop implementation
bench-naive: benchmark.o sgemm-naive.o
	$(CC) -o $@ $(LIBS) benchmark.o sgemm-naive.o $(GOTOLIB)

bench-small: benchmark.o sgemm-small.o
	$(CC) -o $@ $(LIBS) benchmark.o sgemm-small.o $(GOTOLIB)

bench-all: benchmark.o sgemm-all.o
	$(CC) -o $@ $(LIBS) benchmark.o sgemm-all.o $(GOTOLIB)

bench-openmp: benchmark.o sgemm-openmp.o
	$(CC) -o $@ $(LIBS) $(OMP) benchmark.o sgemm-openmp.o $(GOTOLIB)

%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $<

clean:
	rm -f *~ bench-all bench-naive bench-small bench-openmp *.o
