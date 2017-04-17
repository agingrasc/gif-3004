LIBS = -lasound -lvorbis -lvorbisenc -lm -logg

.PHONY: all clean
all: server client bluetooth_sim

%.o: %.c
	gcc -c $< -o $@

server: server.o fifo.o
	gcc server.o fifo.o -Wall $(LIBS) -o $@

client: client.o fifo.o stb_vorbis.c
	gcc client.o fifo.o stb_vorbis.c -Wall $(LIBS) -o $@

bluetooth_sim: bluetooth_sim.o fifo.o
	gcc bluetooth_sim.o fifo.o -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f server
	-rm -f client
	-rm -f bluetooth_sim
