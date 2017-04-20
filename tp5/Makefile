LIBS = -lasound -lvorbis -lvorbisenc -lm -logg
PROFILING= -pg -fprofile-arcs -ftest-coverage

.PHONY: all clean
all: server client bluetooth_sim

%.o: %.c
	gcc $(PROFILING) -c $< -o $@

server: server.o fifo.o
	gcc server.o fifo.o -Wall $(PROFILING) $(LIBS) -o $@

client: client.o fifo.o stb_vorbis.c
	gcc client.o fifo.o stb_vorbis.c -Wall $(PROFILING) $(LIBS) -o $@

bluetooth_sim: bluetooth_sim.o fifo.o
	gcc bluetooth_sim.o fifo.o -Wall $(PROFILING) $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f server
	-rm -f client
	-rm -f bluetooth_sim
