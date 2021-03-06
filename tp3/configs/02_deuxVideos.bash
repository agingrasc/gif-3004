#!/bin/bash

# Suppose être dans le même répertoire que celui contenant les fichiers exécutables
sudo rm /dev/shm/mem*           # On retire les identifiants des zones mémoire partagées des précédentes exécutions
sudo ./decodeur 240p/02_Sintel.ulv /mem1 &
sudo ./decodeur 240p/01_ToS.ulv /mem2 &
sudo ./compositeur /mem1 /mem2 &
wait;

