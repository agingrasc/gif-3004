
#include "fifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 1

int main(int argc, char *argv[]){

    if (argc != 3){
        printf("Two arguments needed");
        exit(1);
    }
    int reader = setr_fifo_reader("/tmp/bluetooth_in");
    int writer = setr_fifo_writer("/tmp/bluetooth_out");

    char buffer[BUFFER_SIZE];

    while (1){
        if (read(reader, buffer, BUFFER_SIZE) != -1)
            write(writer, buffer, BUFFER_SIZE);
        usleep(0);
    }

}
