/*****************************************************************************
*
* Compositeur, laboratoire 3, SETR
*
* Récupère plusieurs flux vidéos à partir d'espaces mémoire partagés et les
* affiche directement dans le framebuffer de la carte graphique.
* 
* IMPORTANT : CE CODE ASSUME QUE TOUS LES FLUX QU'IL REÇOIT SONT EN 427x240
* (427 pixels en largeur, 240 en hauteur). TOUTE AUTRE TAILLE ENTRAÎNERA UN
* COMPORTEMENT INDÉFINI. Les flux peuvent comporter 1 ou 3 canaux. Dans ce
* dernier cas, ils doivent être dans l'ordre BGR et NON RGB.
*
* Le code permettant l'affichage est inspiré de celui présenté sur le blog
* Raspberry Compote (http://raspberrycompote.blogspot.ie/2014/03/low-level-graphics-on-raspberry-pi-part_14.html),
* par J-P Rosti, publié sous la licence CC-BY 3.0.
*
* Marc-André Gardner, Février 2016
* Merci à Yannick Hold-Geoffroy pour l'aide apportée pour la gestion
* du framebuffer.
******************************************************************************/

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
//#include <sys/mman.h>

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


int frames_count[] = {0, 0, 0, 0};


// Fonction permettant de récupérer le temps courant sous forme double
double get_time()
{
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return (double)t.tv_sec + (double)(t.tv_usec)*1e-6;
}


// Cette fonction écrit l'image dans le framebuffer, à la position demandée. Cette fonction peut gérer
// l'affichage de 1, 2, 3 ou 4 images sur le même écran, en utilisant la séparation préconisée dans l'énoncé.
// La position (premier argument) doit être un entier inférieur au nombre total d'images à afficher (second argument).
// Le troisième argument est le descripteur de fichier du framebuffer (nommé fbfb dans la fonction main()).
// Le quatrième argument est un pointeur sur le memory map de ce framebuffer (nommé fbd dans la fonction main()).
// Les cinquième et sixième arguments sont la largeur et la hauteur de ce framebuffer.
// Le septième est une structure contenant l'information sur le framebuffer (nommé vinfo dans la fonction main()).
// Le huitième est la longueur effective d'une ligne du framebuffer (en octets), contenue dans finfo.line_length dans la fonction main().
// Le neuvième argument est le buffer contenant l'image à afficher, et les trois derniers arguments ses dimensions.
void ecrireImage(const int position, const int total,
                 int fbfd, unsigned char* fb, size_t largeurFB, size_t hauteurFB, struct fb_var_screeninfo *vinfoPtr, int fbLineLength,
                 const unsigned char *data, size_t largeurSource, size_t hauteurSource, size_t canauxSource){
    static int currentPage = 0;
    static unsigned char* imageGlobale = NULL;
    if(imageGlobale == NULL)
        imageGlobale = (unsigned char*)calloc(fbLineLength*hauteurFB, 1);

    currentPage = (currentPage+1) % 2;
    //currentPage = 0;
    unsigned char *currentFramebuffer = fb + currentPage * fbLineLength * hauteurFB;

    if(position >= total){
        return;
    }

    const unsigned char *dataTraite = data;
    unsigned char* d = NULL;
    if(canauxSource == 1){
        d = (unsigned char*)tempsreel_malloc(largeurSource*hauteurSource*3);
        unsigned int pos = 0;
        for(unsigned int i=0; i < hauteurSource; ++i){
            for(unsigned int j=0; j < largeurSource; ++j){
                d[pos++] = data[i*largeurSource + j];
                d[pos++] = data[i*largeurSource + j];
                d[pos++] = data[i*largeurSource + j];
            }
        }
        dataTraite = d;
    }

    if(total == 1){
        // Une seule image en plein écran
        for(unsigned int ligne=0; ligne < hauteurSource; ligne++){
            memcpy(currentFramebuffer + ligne * fbLineLength, dataTraite + ligne * largeurSource * 3, largeurFB * 3);
        }
    }
    else if(total == 2){
        // Deux images
        if(position == 0){
            // Image du haut
            for(unsigned int ligne=0; ligne < hauteurSource; ligne++){
                memcpy(imageGlobale + ligne * fbLineLength, dataTraite + ligne * largeurSource * 3, largeurFB * 3);
            }
        }
        else{
            // Image du bas
            for(unsigned int ligne=hauteurSource; ligne < hauteurSource*2; ligne++){
                memcpy(imageGlobale + ligne * fbLineLength, dataTraite + (ligne-hauteurSource) * largeurSource * 3, largeurFB * 3);
            }
        }
    }
    else if(total == 3 || total == 4){
        // 3 ou 4 images
        off_t offsetLigne = 0;
        off_t offsetColonne = 0;
        switch (position) {
            case 0:
                // En haut, à gauche
                break;
            case 1:
                // En haut, à droite
                offsetColonne = largeurSource;
                break;
            case 2:
                // En bas, à gauche
                offsetLigne = hauteurSource;
                break;
            case 3:
                // En bas, à droite
                offsetLigne = hauteurSource;
                offsetColonne = largeurSource;
                break;
        }
        // On copie les données ligne par ligne
        offsetLigne *= fbLineLength;
        offsetColonne *= 3;
        for(unsigned int ligne=0; ligne < hauteurSource; ligne++){
            memcpy(imageGlobale + offsetLigne + offsetColonne, dataTraite + ligne * largeurSource * 3, largeurSource * 3);
            offsetLigne += fbLineLength;
        }
    }

    if(total > 1)
        memcpy(currentFramebuffer, imageGlobale, fbLineLength*hauteurFB);

    if(canauxSource == 1)
        tempsreel_free(d);

    vinfoPtr->yoffset = currentPage * vinfoPtr->yres;
    vinfoPtr->activate = FB_ACTIVATE_VBL;
    if (ioctl(fbfd, FBIOPAN_DISPLAY, vinfoPtr)) {
        printf("Erreur lors du changement de buffer (double buffering inactif)!\n");
    }

    frames_count[position]++;
}



int main(int argc, char* argv[])
{
    int nbrActifs = 0;      // Après votre initialisation, cette variable DOIT contenir le nombre de flux vidéos actifs (de 1 à 4 inclusivement).
    int core = -1;
    int opt;
    while ((opt = getopt(argc, argv, "a:")) != -1) {
        switch (opt) {
            case 'a':
                core = atoi(optarg);
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-a core] flux_entree1 [flux_entree2] [flux_entree3] [flux_entree4]\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    nbrActifs = argc - optind;

    if (nbrActifs < 1 || nbrActifs > 4){
        fprintf(stderr, "Usage: %s [-a core] flux_entree1 [flux_entree2] [flux_entree3] [flux_entree4]\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    struct memPartage zones[nbrActifs];
    for(int i = 0; i<nbrActifs; i++){
        if(initMemoirePartageeLecteur(argv[i+optind], &zones[i]) != 0)
            exit(EXIT_FAILURE);
    }


    // Initialisation des structures nécessaires à l'affichage
    long int screensize = 0;
    // Ouverture du framebuffer
    int fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
        perror("Erreur lors de l'ouverture du framebuffer ");
        return -1;
    }

    // Obtention des informations sur l'affichage et le framebuffer
    struct fb_var_screeninfo vinfo;
    struct fb_var_screeninfo orig_vinfo;
    struct fb_fix_screeninfo finfo;
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Erreur lors de la requete d'informations sur le framebuffer ");
    }

    // On conserve les précédents paramètres
    memcpy(&orig_vinfo, &vinfo, sizeof(struct fb_var_screeninfo));

    // On choisit la bonne résolution
    vinfo.bits_per_pixel = 24;
    switch (nbrActifs) {
        case 1:
            vinfo.xres = 427;
            vinfo.yres = 240;
            break;
        case 2:
            vinfo.xres = 427;
            vinfo.yres = 480;
            break;
        case 3:
        case 4:
            vinfo.xres = 854;
            vinfo.yres = 480;
            break;
        default:
            printf("Nombre de sources invalide!\n");
            return -1;
            break;
    }

    vinfo.xres_virtual = vinfo.xres;
    vinfo.yres_virtual = vinfo.yres * 2;


    if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo)) {
        perror("Erreur lors de l'appel a ioctl ");
    }


    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Erreur lors de la requete d'informations sur le framebuffer ");
    }
    printf("x: %u, y: %u\n", vinfo.grayscale, vinfo.bits_per_pixel);

    // On récupère les "vraies" paramètres du framebuffer
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
        perror("Erreur lors de l'appel a ioctl (2) ");
    }

    int page_size = finfo.line_length * vinfo.yres;

    // On fait un mmap pour avoir directement accès au framebuffer
    screensize = finfo.smem_len;
    unsigned char *fbp = (unsigned char*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);

    if (fbp == MAP_FAILED) {
        perror("Erreur lors du mmap de l'affichage ");
        return -1;
    }

    struct timeval last_frame_time[nbrActifs];
    struct timeval frame_diff[nbrActifs];
    for (int i=0; i<nbrActifs; i++){
        gettimeofday(&last_frame_time[i], NULL);
        frame_diff[i].tv_sec = 0;
        frame_diff[i].tv_usec = 1000000/zones[i].header->fps;
    }

    struct timeval last_frame_counter_time;
    gettimeofday(&last_frame_counter_time, NULL);

    //supression du fichier s'il existait deja et on s'assure qu'il existe, on le ferme immediatement puisqu'on a aucune
    //garantie de quand le programme sera arrete
    FILE *log = fopen("stats.txt", "w+");
    fclose(log);
    while(1){
        // Boucle principale du programme

        // N'oubliez pas que toutes les images fournies à ecrireImage() DOIVENT être en
        // 427x240 (voir le commentaire en haut du document).
        //

        struct timeval current_time;
        gettimeofday(&current_time, NULL);

        int delta_t = current_time.tv_sec - last_frame_counter_time.tv_sec;
        if (delta_t > 1) {
            printf("Log des informations de video.\n");
            last_frame_counter_time.tv_sec = current_time.tv_sec;
            FILE *log = fopen("stats.txt", "a");

            if (log == NULL) {
                printf("Erreur a l'ouverture du fichier de log.\n");
            }
            printf("Fichier ouvert.\n");

            fseek(log, 0, SEEK_END);
            for (int i = 0; i < nbrActifs; i++) {
                int timestamp = last_frame_counter_time.tv_sec;
                float frame_count = frames_count[i];
                float fps = frame_count/delta_t;
                fprintf(log, "(%d) flux %d: %f\n", timestamp, i, fps);

                frames_count[i] = 0;
            }
            fclose(log);
        }

        for(int i=0; i<nbrActifs; i++){
            struct timeval current_diff;
            timersub(&current_time, &last_frame_time[i], &current_diff);
            if (timercmp(&current_diff, &frame_diff[i], <))
                continue;

            if(attenteLecteurAsync(&zones[i]) != 0)
                continue;

            uint32_t w = zones[i].header->largeur;
            uint32_t h = zones[i].header->hauteur;
            uint32_t c = zones[i].header->canaux;
            size_t tailleDonnees = w*h*c;
            ecrireImage(i,
                        nbrActifs,
                        fbfd,
                        fbp,
                        vinfo.xres,
                        vinfo.yres,
                        &vinfo,
                        finfo.line_length,
                        zones[i].data,
                        w,
                        h,
                        c);


            //printf("w: %u h: %u c: %u\n", w, h, c);
            gettimeofday(&last_frame_time[i], NULL);

            //printf("NEW FRAME!\n");

            zones[i].header->frameReader += 1;

            pthread_mutex_unlock(&zones[i].header->mutex);
        }
    }


    // cleanup
    // Retirer le mmap
    munmap(fbp, screensize);


    // reset the display mode
    if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &orig_vinfo)) {
        printf("Error re-setting variable information.\n");
    }
    // Fermer le framebuffer
    close(fbfd);

    return 0;

}

