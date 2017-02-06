#include <sys/un.h>
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

int ouvrirSocket(int* sockfd, char addrPath[]) {
    //ouverture du socket et mise en cache
    *sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Impossible d'initialiser le socket UNIX");
        return -1;
    }

    struct sockaddr_un sockInfo;
    memset(&sockInfo, 0, sizeof sockInfo);
    sockInfo.sun_family = AF_UNIX;
    strncpy(sockInfo.sun_path, addrPath, sizeof sockInfo.sun_path -1);

    printf("Ouverture du socket\n");

    if (connect(*sockfd, (const struct sockaddr*) &sockInfo, sizeof sockInfo) < 0) {
        perror("Erreur de connexion");
        exit(1);
    }
    printf("Socket ouvert!");
    return 0;
}
