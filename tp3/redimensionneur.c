#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "commMemoirePartagee.h"
#include "allocateurMemoire.h"
#include "utils.h"

int main(int argc, char **argv) {

    int out_width = -1;
    int out_height = -1;
    int mode = -1;
    int opt = 0;
    int optc = 0;
    int core = -1;
    int affinity = 1;
    while ((opt = getopt(argc, argv, "aswhm:")) != -1) {
        optc++;
        switch (opt) {
            case 'a':
                optc++;
                if (argv[optc][0] == 'A') {
                    core = 99;
                    printf("all core affinity\n");
                    break;
                }
                if (argv[optc][0] == 'N') {
                    optc++;
                    core = (argv[optc][0] - 48);
                    affinity = 0;
                    printf("will not run on core %d\n", core);
                    break;
                }
                core = (argv[optc][0] - 48);
                printf("will run exclusively on core %d\n", core);
                break;
            case 's':
                optc++;
                if (strcmp(argv[optc], "NORT\0") == 0) {
                    printf("NORT ord!\n");
                    break;
                }
                if (strcmp(argv[optc], "RR\0") == 0) {
                    printf("RR ord!\n");
                    break;
                }
                if (strcmp(argv[optc], "FIFO\0") == 0) {
                    printf("FIFO ord!\n");
                    break;
                }
                if (strcmp(argv[optc], "DEADLINE\0") == 0) {
                    printf("DEADLINE ord!\n");
                    break;
                }
                printf("Mauvais type d'ordonnanceur.\n");
                exit(EXIT_FAILURE);
            case 'w':
                optc++;
                out_width = strtol(argv[optc], NULL, 10);
                break;
            case 'h':
                optc++;
                out_height = strtol(argv[optc], NULL, 10);
                break;
            case 'm':
                optc++;
                mode = strtol(argv[optc], NULL, 10);
                break;
            default:
                printf("Wrong usage\n");
                fprintf(stderr, "Usage: %s [-a core] [-s ORD_TYPE] flux_entree flux_sortie\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    optc++;

    if (argc - optc != 2 || (out_width == -1 || out_height == -1 || mode == -1)) {
        fprintf(stderr, "Usage: %s [-a core] [-s ORD_TYPE] flux_entree flux_sortie\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* memInId = argv[optc];
    char* memOutId = argv[optc+1];

    int last_writer_count = 0;
    int last_reader_count = 0;

    //init memoire
    struct memPartage memOut, memIn;
    //on attend que la memoire partage avec notre ecrivain soit init et prete
    if (initMemoirePartageeLecteur(memInId, &memIn)) {
        exit(EXIT_FAILURE);
    }
    //on attend sur l'ecrivain
    pthread_mutex_lock(&memIn.header->mutex);

    //read only d'une valeur qu'une fois set ne changera pas
    size_t outSize = out_width * out_height * memIn.header->canaux;

    struct memPartageHeader memHeader;
    memHeader.frameWriter = 0;
    memHeader.frameReader = 0;
    memHeader.hauteur = out_height;
    memHeader.largeur = out_width;
    memHeader.fps = memIn.header->fps;
    memHeader.canaux = memIn.header->canaux;
    prepareMemoire(memIn.tailleDonnees, outSize);

    int err = initMemoirePartageeEcrivain(memOutId, &memOut, outSize, &memHeader);
    if (err) {
        printf("Echec lors de l'init de la memoire ecrivain. \n");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_lock(&memOut.header->mutex);

    int in_height = memIn.header->hauteur;
    int in_width = memIn.header->largeur;
    int canal = memIn.header->canaux;

    //Init de la grille de redim
    ResizeGrid resGrid;
    if (mode == 0) {
        resGrid = resizeNearestNeighborInit(out_height, out_width, in_height, in_width);
    }
    else if (mode == 1) {
        resGrid = resizeBilinearInit(out_height, out_width, in_height, in_width);
    }
    else {
        fprintf(stderr, "Le mode doit etre soit 0 ou 1, check usage!\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if (mode == 0) {
            resizeNearestNeighbor(memIn.data, in_height, in_width, memOut.data, out_height, out_width, resGrid, canal);
        }
        else {
            resizeBilinear(memIn.data, in_height, in_width, memOut.data, out_height, out_width, resGrid, canal);
        }

        //on relache la memoire en lecture
        last_writer_count = memIn.header->frameWriter;
        memIn.header->frameReader++;
        pthread_mutex_unlock(&memIn.header->mutex);


        //on relache la memoire en ecriture
        last_reader_count = memOut.header->frameReader;
        memOut.header->frameWriter++;
        pthread_mutex_unlock(&memOut.header->mutex);

        //on attend de se faire signaler par l'ecrivain precedent qu'une modification a ete faite
        while (last_writer_count == memIn.header->frameWriter);
        pthread_mutex_lock(&memIn.header->mutex);
        while (last_reader_count == memOut.header->frameReader);
        pthread_mutex_lock(&memOut.header->mutex);
    }

}
