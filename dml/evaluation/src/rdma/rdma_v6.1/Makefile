.PHONY: clean

CFLAGS  := -Wall -std=c++11 -Os
LD      := g++
LDLIBS  := ${LDLIBS} -lrdmacm -libverbs -lpthread

APPS    :=  RDMABenchmark

HEADE   := RDMABase.h RDMAServer.h RDMAClient.h

all: ${APPS}

RDMABenchmark: RDMABenchmark.o  RDMABase.o RDMAServer.o RDMAClient.o
	${LD} -o $@ $^ ${LDLIBS}

rdma: rdma.o
	${LD} -o $@ $^ ${LDLIBS}

%.o:%.cpp $(HEADE)
	$(LD) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o ${APPS} *.txt

server:
	./RDMABenchmark --server 12.12.12.111
client:
	./RDMABenchmark --server 12.12.12.111 --client 12.12.12.112

syn:
	scp newplan@12.12.11.18:/home/newplan/rdma_test/src/Makefile ./
	scp newplan@12.12.11.18:/home/newplan/rdma_test/src/rdma.cpp ./
	scp newplan@12.12.11.18:/home/newplan/rdma_test/src/rdma.h ./


