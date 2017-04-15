#!/bin/bash

gcc -o server -lasound -lvorbis -lvorbisenc -lm -logg server.c
