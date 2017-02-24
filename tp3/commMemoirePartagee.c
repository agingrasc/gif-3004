#include "commMemoirePartagee.h"

// Appelé au début du programme pour l'initialisation de la zone mémoire (cas du lecteur)
int initMemoirePartageeLecteur(const char* identifiant,
                                struct memPartage *zone){
    
    
    while ((zone->fd = shm_open(identifiant, O_RDONLY, 0444)) < 0);
    struct stat s;
    do{
        fstat(zone->fd, &s);
    }while(s.st_size == 0);

}

// Appelé au début du programme pour l'initialisation de la zone mémoire (cas de l'écrivain)
int initMemoirePartageeEcrivain(const char* identifiant,
                                struct memPartage *zone,
                                size_t taille,
                                struct memPartageHeader* headerInfos){

    if ((zone->fd = shm_open(identifiant, O_RDWR | O_CREAT, 0666)) < 0)
        return -1;

    ftruncate(zone->fd, taille);
    
    void* shm = mmap(NULL, taille, PROT_READ | PROT_WRITE, MAP_SHARED, zone->fd, 0);

    zone->data = (((unsigned char*)shm)+sizeof(memPartageHeader));

    memcpy(zone->header, headerInfos, sizeof(memPartageHeader));
    zone->header->mutex = headerInfos->mutex;
    zone->tailleDonnees = taille;
    zone->copieCompteur = 0;

}

// Appelé par le lecteur pour se mettre en attente d'un résultat
int attenteLecteur(struct memPartage *zone){

}

// Fonction spéciale similaire à attenteLecteur, mais asynchrone : cette fonction ne bloque jamais.
// Cela est utile pour le compositeur, qui ne doit pas bloquer l'entièreté des flux si un seul est plus lent.
int attenteLecteurAsync(struct memPartage *zone){

}

// Appelé par l'écrivain pour se mettre en attente de la lecture du résultat précédent par un lecteur
int attenteEcrivain(struct memPartage *zone){

}

