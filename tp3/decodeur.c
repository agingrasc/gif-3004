#include "allocateurMemoire.h"
#include "commMemoirePartagee.h"

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
    char data[4];
    int idx = 0;
    while (data[idx] = getc(file) != NULL && idx < 4) {
        idx++;
    }

    if (idx < 3) {
        printf("Erreur lecture info video");
        exit(1);
    }

    memcpy(&arg, data, 4);
    printf("arg read: %d\n", arg);
    return arg;
}

int main(int argc, char *argv[]) {

    int err, current_idx;
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
    size_t mem_size = ftell(video_file) - 20;
    rewind(video_file);

    //init memoire ecrivain
    memPartage mem;
    memPartageHeader memHeader;
    err = initMemoirePartageeEcrivain(output_flux, &mem, mem_size, &memHeader);
    if (err) {
        printf("Echec init memoire partage ecrivain (decodeur) pour %s.", filename);
        return 1;
    }

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

    current_idx = 4; //nbr de char lu dans la video

    //mettre le fichier en memoire
    char data;
    current_idx = 0;
    fseek(video_file, 0L, 0);
    while (data = getc(video_file) != EOF) {
        mem.data[current_idx] = data;
        current_idx++;
    }
    fclose(video_file);

    //boucle continu
    while (1) {
        //extraire taille prochaine image
        int image_size = (int) -32;

        //decoder
        //jpgd::decompress_jpeg_image_from_memory(video, image_size, &width, &height, &actual_comp, req_comp);
        break;
    }
    return 0;
}
