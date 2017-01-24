#ifndef ACTIONS_H
#define ACTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Pour recuperer les descriptions d'erreur
#include <errno.h>

// Multiprocessing
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>

// Sockets UNIX
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

#include "communications.h"
#include "requete.h"
#include "telechargeur.h"


// Temps d'attente pour chaque opération bloquante dans la boucle avant de passer à autre chose
#define SLEEP_TIME 1000         // en microsecondes


// Ces fonctions correspondent aux fonctions à exécuter dans la boucle principale du programme
// Voyez le fichier action.c pour une implémentation partielle de certaines de ces fonctions
// ainsi que des explications plus élaborés.

// Toutes ces fonctions prennent la liste des requêtes comme premier argument, suivi du
// nombre maximal de requêtes (autrement dit, la taille du tableau de requêtes).
// verifierNouvelleConnexion() requiert également le descripteur du socket comme troisième argument.
// Ces arguments sont suffisants : vous ne devriez pas avoir à utiliser d'autres variables globales
// ou à passer des arguments supplémentaires.

int verifierNouvelleConnexion(struct requete reqList[], int maxlen, int socket);
int traiterConnexions(struct requete reqList[], int maxlen);
int traiterTelechargements(struct requete reqList[], int maxlen);

#endif
