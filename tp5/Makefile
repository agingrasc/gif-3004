LIBS = -lasound -lvorbis -lvorbisenc -lm -logg
PROFILING= -pg -fprofile-arcs -ftest-coverage
OPTIMIZATION= -O3

.PHONY: all clean
all: server client bluetooth_sim

%.o: %.c
	gcc $(OPTIMIZATION) $(PROFILING) -c $< -o $@

server: server.o fifo.o
	gcc server.o fifo.o $(OPTIMIZATION) -Wall $(PROFILING) $(LIBS) -o $@

client: client.o fifo.o stb_vorbis.c
	gcc client.o fifo.o stb_vorbis.c $(OPTIMIZATION) -Wall $(PROFILING) $(LIBS) -o $@

bluetooth_sim: bluetooth_sim.o fifo.o
	gcc bluetooth_sim.o fifo.o $(OPTIMIZATION) -Wall $(PROFILING) $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f server
	-rm -f client
	-rm -f bluetooth_sim
