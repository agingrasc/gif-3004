LIBS = -lasound -lvorbis -lvorbisenc -lm -logg
#PROFILING= -pg -fprofile-arcs -ftest-coverage
PROFILING= 
#OPTIMIZATION= -O3
OPTIMIZATION= -Og

.PHONY: all clean
all: audio_in bluetooth_server filters client bluetooth_sim

%.o: %.c
	gcc $(OPTIMIZATION) $(PROFILING) -c $< -o $@

audio_in: audio_in.o fifo.o
	gcc audio_in.o fifo.o $(OPTIMIZATION) -Wall $(PROFILING) $(LIBS) -o $@

filters: filters.o fifo.o
	gcc filters.o fifo.o $(OPTIMIZATION) -Wall $(PROFILING) $(LIBS) -o $@

bluetooth_server: bluetooth_server.o fifo.o
	gcc bluetooth_server.o fifo.o $(OPTIMIZATION) -Wall $(PROFILING) $(LIBS) -o $@

client: client.o fifo.o stb_vorbis.c
	gcc client.o fifo.o stb_vorbis.c $(OPTIMIZATION) -Wall $(PROFILING) $(LIBS) -o $@

bluetooth_sim: bluetooth_sim.o fifo.o
	gcc bluetooth_sim.o fifo.o $(OPTIMIZATION) -Wall $(PROFILING) $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f audio_in
	-rm -f bluetooth_server
	-rm -f filters
	-rm -f client
	-rm -f bluetooth_sim
