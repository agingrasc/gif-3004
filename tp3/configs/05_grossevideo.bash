#!/bin/bash

# Suppose être dans le même répertoire que celui contenant les fichiers exécutables
sudo rm /dev/shm/mem*           # On retire les identifiants des zones mémoire partagées des précédentes exécutions
sudo ./decodeur 720p/02_Sintel.ulv /mem1 &
sudo ./redimensionneur -w 427 -h 240 -m 0 /mem1 /mem2 &
sudo ./compositeur /mem2 &
wait;

