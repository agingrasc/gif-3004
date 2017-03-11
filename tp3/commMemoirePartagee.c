#include "commMemoirePartagee.h"

// Appelé au début du programme pour l'initialisation de la zone mémoire (cas du lecteur)
int initMemoirePartageeLecteur(const char* identifiant,
                                struct memPartage *zone){
    
    
    while ((zone->fd = shm_open(identifiant, O_RDWR, 0666)) < 0);
    struct stat s;
    do{
        fstat(zone->fd, &s);
    }while(s.st_size == 0);

    void* shm = mmap(NULL, s.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, zone->fd, 0);
    memPartageHeader* header = (memPartageHeader*)shm;

    zone->data = (((unsigned char*)shm)+sizeof(memPartageHeader));
    zone->header = header;
    zone->tailleDonnees = s.st_size - sizeof(memPartageHeader);
    zone->copieCompteur = 0;

    while(zone->header->frameWriter == 0);

    return 0;

}

// Appelé au début du programme pour l'initialisation de la zone mémoire (cas de l'écrivain)
int initMemoirePartageeEcrivain(const char* identifiant,
                                struct memPartage *zone,
                                size_t taille,
                                struct memPartageHeader* headerInfos){

    if ((zone->fd = shm_open(identifiant, O_RDWR | O_CREAT, 0666)) < 0)
        return -1;

    ftruncate(zone->fd, taille+sizeof(memPartageHeader));
    
    void* shm = mmap(NULL, taille, PROT_READ | PROT_WRITE, MAP_SHARED, zone->fd, 0);

    memPartageHeader* header = (memPartageHeader*)shm;

    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);

    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);

    pthread_mutex_init(&header->mutex, &mutex_attr);

    header->largeur = headerInfos->largeur;
    header->hauteur = headerInfos->hauteur;
    header->canaux = headerInfos->canaux;
    header->fps = headerInfos->fps;
    header->frameReader = 0;
    header->frameWriter = 0;

    zone->data = (((unsigned char*)shm)+sizeof(memPartageHeader));
    zone->header = header;
    zone->tailleDonnees = taille;
    zone->copieCompteur = 99999;

    return 0;

}

// Appelé par le lecteur pour se mettre en attente d'un résultat
int attenteLecteur(struct memPartage *zone){
    while(zone->header->frameWriter == zone->header->frameReader); //Might not be the right condition
    return pthread_mutex_lock(&zone->header->mutex);
}

// Fonction spéciale similaire à attenteLecteur, mais asynchrone : cette fonction ne bloque jamais.
// Cela est utile pour le compositeur, qui ne doit pas bloquer l'entièreté des flux si un seul est plus lent.
int attenteLecteurAsync(struct memPartage *zone){
    if(zone->header->frameWriter == zone->header->frameReader)
        return 1;

    return pthread_mutex_trylock(&zone->header->mutex);
    
}

// Appelé par l'écrivain pour se mettre en attente de la lecture du résultat précédent par un lecteur
int attenteEcrivain(struct memPartage *zone){
    while(zone->copieCompteur == zone->header->frameReader);
    return pthread_mutex_lock(&zone->header->mutex);
}

