#!/bin/bash

arm-linux-gnueabihf-gcc --sysroot=../../../dump -Wl,--sysroot=../../../dump,-rpath=../../../dump/lib/arm-linux-gnueabihf,-rpath=../../../dump/usr/lib/arm-linux-gnueabihf -L../../../dump/usr/lib/arm-linux-gnueabihf -L../../../dump/lib -lfuse -D_FILE_OFFSET_BITS=64 -I../../../dump/usr/include  *.c -o daemonFusePi
