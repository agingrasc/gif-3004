#!/bin/bash

g++ -lrt decodeur.c commMemoirePartagee.c allocateurMemoire.c jpgd.cpp utils.c -o decodeur
g++ -lrt compositeur.c commMemoirePartagee.c allocateurMemoire.c utils.c -o compositeur
