#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>

#include <stropts.h>

#include <sys/time.h>
#include <sys/resource.h>

#include <sys/types.h>

// Allocation mémoire, mmap et mlock
#include <sys/mman.h>

// Gestion des ressources et permissions
#include <sys/resource.h>

// Mesure du temps
#include <time.h>

// Obtenir la taille des fichiers
#include <sys/stat.h>

// Contrôle de la console
#include <linux/fb.h>
#include <linux/kd.h>

// Gestion des erreurs
#include <err.h>
#include <errno.h>

#include "allocateurMemoire.h"
#include "commMemoirePartagee.h"
#include "utils.h"

#define MUCH 0

int main(int argc, char* argv[])
{
    int core = -1;
    void (*filter_function) (const unsigned int height, const unsigned int width,
                             const unsigned char *input, unsigned char *output,
                             const unsigned int kernel_size, float sigma,
                             const unsigned int n_channels);
    int opt;
    while ((opt = getopt(argc, argv, "a:t:")) != -1) {
        switch (opt) {
            case 'a':
                core = strtol(optarg, NULL, 10);
                break;
            case 't':
                if (strtol(optarg, NULL, 10) == 0){
                    filter_function = lowpassFilter;
                }
                else{
                    filter_function = highpassFilter;
                }
                break;
            default: /* '?' */
                        fprintf(stderr, "Usage: %s [-a core] [-t type_de_filtre] flux_entree1 flux_sortie\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }


    if ((argc - optind) != 2){
        fprintf(stderr, "Usage: %s [-a core] [-t type_de_filtre] flux_entree1 flux_sortie\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    struct memPartage zone_lecture;
    struct memPartage zone_ecriture;

    int reader_count = 0;
    int writer_count = 0;

    printf("Init lecteur: %s\n", argv[optind]);
    if(initMemoirePartageeLecteur(argv[optind], &zone_lecture) != 0)
        exit(EXIT_FAILURE);
    while (writer_count == zone_lecture.header->frameWriter);
    pthread_mutex_lock(&zone_lecture.header->mutex);

    uint32_t w = zone_lecture.header->largeur;
    uint32_t h = zone_lecture.header->hauteur;
    uint32_t c = zone_lecture.header->canaux;
    size_t tailleDonnees = w*h*c;
    prepareMemoire(zone_lecture.tailleDonnees, 2*tailleDonnees);

    printf("w: %u, h: %u, c: %u\n", w, h, c);

    if(initMemoirePartageeEcrivain(argv[optind+1], &zone_ecriture, tailleDonnees, zone_lecture.header) != 0)
        exit(EXIT_FAILURE);
    pthread_mutex_lock(&zone_ecriture.header->mutex);


    while(1){

        // liberation zone_lecture
        zone_lecture.header->frameReader += 1;
        writer_count = zone_lecture.header->frameWriter;
        pthread_mutex_unlock(&zone_lecture.header->mutex);

        filter_function(h, w, zone_lecture.data, zone_ecriture.data, 5, 10, c);

        // liberation zone_ecriture
        zone_ecriture.header->frameWriter += 1;
        zone_ecriture.copieCompteur = zone_ecriture.header->frameReader;

        reader_count = zone_ecriture.header->frameReader;
        pthread_mutex_unlock(&zone_ecriture.header->mutex);
        while (writer_count == zone_lecture.header->frameWriter);
        pthread_mutex_lock(&zone_lecture.header->mutex);
        while (reader_count == zone_ecriture.header->frameReader);
        pthread_mutex_lock(&zone_ecriture.header->mutex);
    }

    return 0;

}

