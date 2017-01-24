#include "communications.h"

int envoyerMessage(int socket, void *header, char* payload){
    struct msgRep *headerS = (struct msgRep*)header;
    int octetsEcrits = write(socket, (char*)headerS, sizeof(struct msgRep));
    if(octetsEcrits < sizeof(struct msgRep)){
        printf("Erreur lors de l'envoi du header : %i octets ecrits sur %i\n", octetsEcrits, sizeof(struct msgRep));
        return -1;
    }

    unsigned int totalEnvoi = 0;
    while(totalEnvoi < headerS->sizePayload){
        octetsEcrits = write(socket, payload, headerS->sizePayload - totalEnvoi);
        if(octetsEcrits < 0){
            printf("Erreur lors de l'envoi du payload : %i octets ecrits sur %i (derniere ecriture : %i)\n", totalEnvoi, headerS->sizePayload, octetsEcrits);
            return octetsEcrits;
        }
        totalEnvoi += octetsEcrits;
        if(VERBOSE)
            printf("SEND LOOP : %u\n", octetsEcrits);
    }
    return totalEnvoi + sizeof(struct msgRep);
}
