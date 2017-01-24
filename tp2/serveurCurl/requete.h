#ifndef REQUETE_H
#define REQUETE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "communications.h"

// Les différents statuts que peut prendre une requête
#define REQ_STATUS_INACTIVE 0       // Aucune requête effective en cours (pas de socket lié, pas de pipe lié)
#define REQ_STATUS_LISTEN 1         // Une connexion a été demandée sur le socket fdSocket, on attend de recevoir une requête de la part du client
#define REQ_STATUS_INPROGRESS 2     // La requête a été reçue et un processus enfant a été créé; le téléchargement est en cours
#define REQ_STATUS_READYTOSEND 3    // Le téléchargement est terminé et le processus enfant a quitté, on est prêt à envoyer les données au client

// Structure contenant les informations sur une requête
struct requete{
        int status;         // Status de cette requête (voir les #define plus haut)
        int fdSocket;       // Si un client s'est connecté, descripteur de fichier du socket
        int fdPipe;         // Si un processus enfant est lancé, descripteur de fichier du bout de lecture du pipe
        pid_t pid;          // Si un processus enfant est lancé, ID de ce processus
        char* buf;          // Si les données sont prêtes, pointe vers le buffer contenant ces données
        size_t len;         // Si les données sont prêtes, contient la longueur des données (en octets)
};

// Recherche une entrée inoccupée dans la table des connexions passée en argument et retourne son index.
// Retourne -1 si aucune entree n'est libre (autrement dit, on ne peut pas accepter une nouvelle requête).
// Le second argument est le nombre d'entrées au total (occupées ou non) dans la table de connexions
int nouvelleRequete(struct requete reqList[], int lenreq);

// Cette fonction parcourt la liste des requêtes pour déterminer si certaines sont prêtes à être renvoyées
// au client (si leur statut est REQ_STATUS_READYTOSEND). Si c'est le cas, elle les envoie, ferme le socket
// et retire la requête de la liste des requêtes (en libérant donc une place pour une nouvelle requête).
// Elle retourne le nombre de requêtes qu'elle a terminé.
// Le second argument est le nombre d'entrées au total (occupées ou non) dans la table de connexions
int envoyerReponses(struct requete reqList[], int lenreq);

#endif
