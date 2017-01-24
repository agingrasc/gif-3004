#ifndef TELECHARGEUR_H
#define TELECHARGEUR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// cURL (pour le téléchargement de fichiers)
#include <curl/curl.h>

// Pour la définition des structures de communication
#include "communications.h"

struct MemoryStructCurl {
  char *memory;
  size_t size;
};

// Fonction utilisée à l'interne, vous n'avez pas à l'utiliser directement
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

// Reçoit deux arguments :
// pipeFd est le descripteur de fichier (file descriptor) du pipe utilisé pour retourner le fichier téléchargé
// reqBuffer est un buffer contenant la requête envoyée par le client (y compris l'en-tête)
//
// Cette fonction télécharge le fichier demandé et l'écrit sur le pipe. Le contenu du fichier est précédé d'une
// valeur de la taille de size_t qui contient le nombre d'octets qui seront écrits par la suite dans le pipe.
// Par exemple, si le fichier fait 1865 octets, alors cette fonction enverra d'abord 1865 sur le pipe, en utilisant
// sizeof(size_t) octets pour le faire. Par la suite, elle enverra les 1865 octets composant le fichier.
//
// Notez finalement que si une erreur s'est produite lors du téléchargement, la taille renvoyée sera de 0, et aucune
// donnée ne sera envoyée par la suite.
void executeRequete(int pipeFd, char* reqBuffer);

#endif
