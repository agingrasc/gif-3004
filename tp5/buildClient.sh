#!/bin/bash

gcc -o client -lasound -lvorbis -lvorbisenc client.c stb_vorbis.c
