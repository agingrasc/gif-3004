#!/bin/bash

# Suppose être dans le même répertoire que celui contenant les fichiers exécutables
sudo rm /dev/shm/mem*           # On retire les identifiants des zones mémoire partagées des précédentes exécutions
sudo ./decodeur 480p/03_BigBuckBunny.ulv /mem1 &
sudo ./redimensionneur -w 427 -h 240 -m 1 /mem1 /mem2 &
sudo ./decodeur 480p/05_Llama_drama.ulv /mem3 &
sudo ./redimensionneur -w 427 -h 240 -m 1 /mem3 /mem4 &
sudo ./decodeur 480p/01_ToS.ulv /mem5 &
sudo ./redimensionneur -w 427 -h 240 -m 1 /mem5 /mem6 &
sudo ./compositeur /mem2 /mem4 /mem5 /mem6 &
wait;

