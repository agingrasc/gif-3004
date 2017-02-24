#include "allocateurMemoire.h"

// Resize : 2*image (3 canaux) + 2*image (1 canal)
// Low-pass : 2*image (3 canaux)
// High-pass : 3*image (3 canaux)

#define ALLOC_N_BIG 16
#define ALLOC_N_SMALL 100
#define ALLOC_SMALL_SIZE 1024

int prepareMemoire(size_t tailleImageEntree, size_t tailleImageSortie){

}

void* tempsreel_malloc(size_t taille){
    return malloc(taille);
}

void tempsreel_free(void* ptr){
    free(ptr);
}

