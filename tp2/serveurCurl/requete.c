#include "requete.h"

int nouvelleRequete(struct requete reqList[], int lenreq){
    // Cette fonction est déjà implémentée pour vous
    for(int i=0; i < lenreq; i++){
        if(reqList[i].status == REQ_STATUS_INACTIVE)
            return i;
    }
    return -1;
}

int envoyerReponses(struct requete reqList[], int lenreq){
    // Cette fonction est déjà implémentée pour vous
    int compteur = 0, octetsTraites;
    for(int i = 0; i < lenreq; i++){
        if(reqList[i].status == REQ_STATUS_READYTOSEND){
            // On cree le header (indiquant la taille)
            struct msgRep repHeader;
            repHeader.sizePayload = reqList[i].len;
            if(repHeader.sizePayload == 0){
                // Erreur lors du telechargement
                repHeader.status = STATUS_ERREUR_TELECHARGEMENT;
            }
            else{
                repHeader.status = STATUS_OK;
            }
            if(VERBOSE)
                printf("Taille envoyee %u\n", repHeader.sizePayload);

            octetsTraites = envoyerMessage(reqList[i].fdSocket, &repHeader, reqList[i].buf);

            // On libere la memoire et on designe le slot comme libre
            // pour accueillir une nouvelle connexion
            if(repHeader.sizePayload != 0){
                free(reqList[i].buf);           // S'il y a eu une erreur, aucun buffer n'a ete alloue
            }
            close(reqList[i].fdSocket);
            memset(reqList+i, 0, sizeof(struct requete));
            reqList[i].status = REQ_STATUS_INACTIVE;
            compteur++;
        }
    }
    return compteur;
}
