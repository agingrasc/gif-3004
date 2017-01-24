#ifndef COMM_H
#define COMM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Afficher plus d'informations de debug sur la console
#define VERBOSE 1

// Les deux types de requêtes que l'on peut recevoir
#define REQ_LIST 1          // Retourner le contenu du fichier index.txt
#define REQ_READ 2          // Retourner le contenu du fichier passé dans la suite de la requête

// Définit l'en-tête d'une requête
struct msgReq{
    char type;            // type de la requête
    size_t sizePayload;   // taille du reste de la requête (dans le cas d'un nom, la taille du buffer contenant ce dernier)
};


// Les différents status que le serveur peut retourner comme réponse à une requête
#define STATUS_OK 0                     // Requête exécutée avec le succès, le contenu du fichier suit
#define STATUS_REQ_INVALIDE 1           // La requête est invalide (par exemple parce qu'aucun nom de fichier n'est donné)
#define STATUS_TYPE_REQ_INCONNU 2       // Le type de requête est inconnu
#define STATUS_ERREUR_TELECHARGEMENT 3  // Le fichier n'existe pas sur le serveur
                                        // De manière générale, c'est cette dernière erreur que vous rencontrerez le plus souvent
// Définit l'en-tête d'une réponse
struct msgRep{
    char status;        // statut de la requête (voir plus haut)
    size_t sizePayload; // taille du fichier qui suit, en octets
};

// Envoie sur le socket passé en argument le message composé de l'en-tête 'header'
// et son contenu. La même fonction est utilisée pour les deux types de headers, puisqu'ils ont la même taille.
// Les champs status/type et sizePayload de l'en-tête doivent déjà avoir été assignés à la bonne valeur
// Retourne le nombre d'octets écrits au total, ou le code d'erreur (négatif) s'il y a lieu
int envoyerMessage(int socket, void *header, char* payload);

#endif
