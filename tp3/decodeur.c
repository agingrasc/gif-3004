#include <getopt.h>
#include "allocateurMemoire.h"
#include "commMemoirePartagee.h"
#include "jpgd.h"
#include "utils.h"

// Gestion des ressources et permissions

// Définition de diverses structures pouvant vous être utiles pour la lecture d'un fichier ULV
#define HEADER_SIZE 4
const char header[] = "SETR";
#define INFO_SIZE 20

struct videoInfos {
    uint32_t largeur;
    uint32_t hauteur;
    uint32_t canaux;
    uint32_t fps;
};

/******************************************************************************
* FORMAT DU FICHIER VIDEO
* Offset     Taille     Type      Description
* 0          4          char      Header (toujours "SETR" en ASCII)
* 4          4          uint32    Largeur des images du vidéo
* 8          4          uint32    Hauteur des images du vidéo
* 12         4          uint32    Nombre de canaux dans les images
* 16         4          uint32    Nombre d'images par seconde (FPS)
* 20         4          uint32    Taille (en octets) de la première image -> N
* 24         N          char      Contenu de la première image (row-first)
* 24+N       4          uint32    Taille (en octets) de la seconde image -> N2
* 24+N+4     N2         char      Contenu de la seconde image
* 24+N+N2    4          uint32    Taille (en octets) de la troisième image -> N2
* ...                             Toutes les images composant la vidéo, à la suite
*            4          uint32    0 (indique la fin du fichier)
******************************************************************************/

/**
 * Valide le header du fichier video reçu
 * @param ptr
 * @return
 */
int validate_header(const char *actual_header) {

    return strncmp(header, actual_header, 4) != 0;
    
}

int main(int argc, char *argv[]) {

    prepareMemoire(0, 0);

    uint32_t current_idx = 0;
    int err = 0;
    // Écrivez le code de décodage et d'envoi sur la zone mémoire partagée ici!
    // N'oubliez pas que vous pouvez utiliser jpgd::decompress_jpeg_image_from_memory()
    // pour décoder une image JPEG contenue dans un buffer!
    // N'oubliez pas également que ce décodeur doit lire les fichiers ULV EN BOUCLE

    //recuperer les arguments: file, flux_sortie
    char *filename;
    char *output_flux;

    int opt;
    int optc = 0; //program filename
    int affinity = 1;
    int core = -1;
    while ((opt = getopt(argc, argv, "as:")) != -1) {
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
            default:
                printf("Wrong usage\n");
                fprintf(stderr, "Usage: %s [-a core] [-s ORD_TYPE] video_file flux_sortie\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    optc++;

    if ((argc - optc) != 2) {
        fprintf(stderr, "Usage: %s [-a core] [-s ORD_TYPE] video_file flux_sortie\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    filename = (argv[optc]);
    optc++;
    output_flux = (argv[optc]);
    printf("Args: %s, %s\n", filename, output_flux);

    int video_fd = open(filename, O_RDONLY);
    if (video_fd == -1) {
        printf("Echec de l'ouverture du fichier video: %s", filename);
        return 1;
    }

    struct stat video_stats;

    fstat(video_fd, &video_stats);

    uint32_t video_size = video_stats.st_size;

    const char* video_mem = (char*)mmap(NULL, video_size, PROT_READ, MAP_POPULATE | MAP_PRIVATE, video_fd, 0);

    //valider header
    err = validate_header(video_mem);
    if (err) {
        printf("Header invalide! %s", filename);
        return 1;
    }

    //extraire meta information (larger, hauteur, req_comp, fps)
    videoInfos video_info;
    memcpy(&video_info.largeur, video_mem + 4, 4);
    memcpy(&video_info.hauteur, video_mem + 8, 4);
    memcpy(&video_info.canaux, video_mem + 12, 4);
    memcpy(&video_info.fps, video_mem + 16, 4);
    printf("Info lu:\nlargeur: %d\nhauteur: %d\ncanaux: %d\nfps: %d\n", video_info.largeur, video_info.hauteur,
           video_info.canaux, video_info.fps);

    
    //init memoire ecrivain
    memPartage mem;
    memPartageHeader memHeader;
    memHeader.frameWriter = 0;
    memHeader.frameReader = 0;
    memHeader.hauteur = 0;
    memHeader.largeur = 0;
    memHeader.fps = 0;
    memHeader.canaux = 0;
    memHeader.hauteur = video_info.hauteur;
    memHeader.largeur = video_info.largeur;
    memHeader.fps = video_info.fps;
    memHeader.canaux = video_info.canaux;

    size_t frame_size = video_info.hauteur * video_info.largeur * video_info.canaux;
    err = initMemoirePartageeEcrivain(output_flux, &mem, frame_size, &memHeader);
    if (err) {
        printf("Echec init memoire partage ecrivain (decodeur) pour %s.", filename);
        return 1;
    }
    pthread_mutex_lock(&mem.header->mutex);
    mem.header->frameWriter = 0;

    //mettre le fichier en memoire
    int data;
    current_idx = INFO_SIZE;
    uint32_t current_reader_idx = 0;
    //boucle continu
    while (1) {
        mem.header->frameWriter++;

        if (current_idx >= video_size - 4) {
            current_idx = INFO_SIZE;
            printf("On reboucle la video");
        }
        //extraire taille prochaine image
        uint32_t compressed_image_size;
        memcpy(&compressed_image_size, video_mem + current_idx, 4);
        current_idx += 4;

        //decoder
        int width, height, actual_comp = 0;
        width = video_info.largeur;
        height = video_info.hauteur;
        unsigned char* frame;

        frame = jpgd::decompress_jpeg_image_from_memory((const unsigned char*) (video_mem + current_idx), compressed_image_size, &width, &height, &actual_comp, video_info.canaux);

        mem.header->largeur = width;
        mem.header->hauteur = height;
        mem.header->canaux = actual_comp;
        if (width != video_info.largeur) {
            printf("La largeur a change selon le decodage (avant, apres, index en char): %d, %d, %d\n", video_info.largeur, width, current_idx);
        }
        if (height != video_info.hauteur) {
            printf("La hauteur a change selon le decodage (avant, apres, index en char): %d, %d, %d\n", video_info.hauteur, height, current_idx);
        }
        if (actual_comp != video_info.canaux) {
            printf("Le nombre de canaux a change selon le decodage (avant, apres, index en char): %d, %d, %d\n", video_info.canaux, actual_comp, current_idx);
        }

        uint32_t image_size = width*height*actual_comp;

        //enregistreImage(frame, height, width, actual_comp, "frame.ppm");

        //copie de la frame
        memcpy(mem.data, frame, image_size);

        tempsreel_free(frame);

        //liberation du mutex et mise a jour de notre index prive
        current_idx += compressed_image_size;
        current_reader_idx = mem.header->frameReader;
        pthread_mutex_unlock(&mem.header->mutex);
        while (current_reader_idx == mem.header->frameReader); //on attend apres le lecteur

        //acquisition du mutex
        pthread_mutex_lock(&mem.header->mutex);
    }
    return 0;
}
