#A simple Makefile

CC = gcc
CCFLAGS = -g -O0
INCS = -I. -I/usr/include
CFLAGS = $(CCFLAGS) $(INCS)
LIBS = -libverbs
DEPS = roce.h
SVROBJS = rocesvr.o roce.o
CLIOBJS = rocecli.o roce.o
DEMOOBJS = example.o

BINDIR=bin
BINDIRCHK=@if [ ! -d "$(BINDIR)" ]; then mkdir -p "$(BINDIR)"; fi

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(BINDIR)/rocesvr $(BINDIR)/rocecli $(BINDIR)/rdma_demo

$(BINDIR)/rocesvr: $(SVROBJS)
	$(BINDIRCHK)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(BINDIR)/rocecli: $(CLIOBJS)
	$(BINDIRCHK)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

$(BINDIR)/rdma_demo: $(DEMOOBJS)
	$(BINDIRCHK)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -rf *.o bin
