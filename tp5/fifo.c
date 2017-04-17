
#include "fifo.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

int setr_fifo_writer(const char *pathname){

    int err;
    if ((err = unlink(pathname)) != 0){
        switch(errno){
            case 0:
            case ENOENT:
                break;
            case EBUSY:
                printf("Pipe already in use\n");
                return -1;
            default:
                return -1;
        }
    }

    mkfifo(pathname, 0666);
    return open(pathname, O_WRONLY);
}

int setr_fifo_reader(const char *pathname){

    int fd = -1;
    printf("Waiting for %s to exist\n", pathname);
    do{
        fd = open(pathname, O_RDONLY);
        usleep(100);
    }while(fd == -1);
    printf("%s opened for reading\n", pathname);
    return fd;
}
