CC      = g++
CFLAGS  = -O3 -Wno-write-strings
CCFLAGS = -O3 -std=gnu++0x
WDEBUG  = -g

all: link tidy

link: graph initialcondition epidemic main
	$(CC) $(WDEBUG) $(CFLAGS) -o bin/simplesir main.o epidemic.o initialcondition.o graph.o

graph:
	$(CC) $(WDEBUG) $(CFLAGS)  -c source/graph.c

initialcondition:
	$(CC) $(WDEBUG) $(CFLAGS)  -c source/initialcondition.c

epidemic:
	$(CC) $(WDEBUG) $(CCFLAGS) -c source/epidemic.cpp

main:
	$(CC) $(WDEBUG) $(CCFLAGS) -c source/main.cpp

tidy:
	rm main.o epidemic.o initialcondition.o graph.o

clean:
	rm -f bin/simplesir