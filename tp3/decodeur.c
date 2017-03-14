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
int validate_header(FILE *file) {
    for (int i = 0; i < HEADER_SIZE; i++) {
        if (getc(file) != header[i]) {
            return 1;
        }
    }

    return 0;
}

uint32_t read_uint32(FILE *file) {
    uint32_t arg = 0;
    int c;
    char data[4] = {0, 0, 0, 0};
    int idx = 0;
    //ordre de la condition important
    while (idx < 4 && (c = fgetc(file)) != EOF) {
        data[idx] = c;
        idx++;
    }

    if (idx < 4) {
        printf("Erreur lecture info video");
        exit(1);
    }

    memcpy(&arg, data, 4);
    return arg;
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
    filename = (argv[1]);
    output_flux = (argv[2]);
    printf("Args: %s, %s\n", filename, output_flux);

    //ouverture fichier video
    FILE *video_file = fopen(filename, "r");
    if (video_file == NULL) {
        printf("Echec de l'ouverture du fichier video: %s", filename);
        return 1;
    }
    fseek(video_file, 0L, SEEK_END);
    size_t video_size = ftell(video_file);
    rewind(video_file);

    //valider header
    err = validate_header(video_file);
    if (err) {
        printf("Header invalide! %s", filename);
        return 1;
    }

    //extraire meta information (larger, hauteur, req_comp, fps)
    videoInfos video_info;
    video_info.largeur = read_uint32(video_file);
    video_info.hauteur = read_uint32(video_file);
    video_info.canaux = read_uint32(video_file);
    video_info.fps = read_uint32(video_file);
    printf("Info lu:\nlargeur: %d\nhauteur: %d\ncanaux: %d\nfps: %d\n", video_info.largeur, video_info.hauteur,
           video_info.canaux, video_info.fps);

    fclose(video_file);
    
    int video_fd = open(filename, O_RDONLY);
    if (video_fd == -1) {
        printf("Echec de l'ouverture du fichier video: %s", filename);
        return 1;
    }

    const unsigned char* video_mem = (unsigned char*)mmap(NULL, video_size, PROT_READ, MAP_POPULATE | MAP_PRIVATE, video_fd, 0);

    printf("dfjd: %p\n", video_mem);

    uint32_t* test = (uint32_t*)video_mem;

    printf("dfjd: %u\n", *test);

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

        printf("Frame: %d\n", mem.header->frameWriter);
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

        printf("e2o\n");
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
