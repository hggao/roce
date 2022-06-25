#A simple Makefile

CC = gcc
CCFLAGS = -g -O0
INCS = -I. -I/usr/include
CFLAGS = $(CCFLAGS) $(INCS)
LIBS = -libverbs
DEPS = roce.h
SVROBJS = rocesvr.o roce.o
CLIOBJS = rocecli.o roce.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: rocesvr rocecli

rocesvr: $(SVROBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

rocecli: $(CLIOBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f *.o rocesvr rocecli
