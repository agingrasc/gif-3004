#!/bin/bash

# Suppose être dans le même répertoire que celui contenant les fichiers exécutables
sudo rm /dev/shm/mem*           # On retire les identifiants des zones mémoire partagées des précédentes exécutions
sudo ./decodeur 320p/03_BigBuckBunny.ulv /mem1 &
sudo ./redimensionneur -w 427 -h 240 -m 1 /mem1 /mem2 &
sudo ./convertisseur /mem2 /mem3 &

sudo ./decodeur 320p/04_Caminandes.ulv /mem4 &
sudo ./convertisseur /mem4 /mem5 &
sudo ./redimensionneur -w 427 -h 240 -m 1 /mem5 /mem6 &

sudo ./compositeur /mem3 /mem6 &
wait;

