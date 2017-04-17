
#include "fifo.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 10

int main(int argc, char *argv[]){

    if (argc != 3){
        printf("Two arguments needed");
        exit(1);
    }
    int writer = setr_fifo_writer("/tmp/bluetooth_out");
    int reader = setr_fifo_reader("/tmp/bluetooth_in");

    char buffer[BUFFER_SIZE];

    while (1){
        int data_read = 0;
        if ((data_read = read(reader, buffer, BUFFER_SIZE)) != -1)
            write(writer, buffer, data_read);
        usleep(0);
    }

}
