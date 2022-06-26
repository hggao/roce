#A simple Makefile

CC = gcc
CCFLAGS = -g -O0
INCS = -I. -I/usr/include
CFLAGS = $(CCFLAGS) $(INCS)
LIBS = -libverbs
DEPS = roce.h
SVROBJS = rocesvr.o roce.o
CLIOBJS = rocecli.o roce.o

BINDIR=bin

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(BINDIR)/rocesvr $(BINDIR)/rocecli

$(BINDIR)/rocesvr: $(SVROBJS)
	if [ ! -d "$(BINDIR)" ]; then mkdir -p "$(BINDIR)"; fi
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(BINDIR)/rocecli: $(CLIOBJS)
	if [ ! -d "$(BINDIR)" ]; then mkdir -p "$(BINDIR)"; fi
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -rf *.o bin
