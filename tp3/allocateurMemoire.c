#include "allocateurMemoire.h"
#include <stdint.h>

// Resize : 2*image (3 canaux) + 2*image (1 canal)
// Low-pass : 2*image (3 canaux)
// High-pass : 3*image (3 canaux)

#define ALLOC_N_BIG 16
#define ALLOC_N_SMALL 100
#define ALLOC_SMALL_SIZE 1024

static void* spaces[4];
static uint8_t libre[4];

int prepareMemoire(size_t tailleImageEntree, size_t tailleImageSortie){

    for(int i = 0; i<4; i++){ 
        spaces[i] = malloc(4194304);
        libre[i] = 1;
    }

}

void* tempsreel_malloc(size_t taille){
    for(int i = 0; i<4; i++){ 
        if(libre[i] == 1){
            libre[i] = 0;
            printf("Alloc %u\n", i);
            return spaces[i];
        }
    }
    
    printf("No memory found %u\n");
    return NULL;
}

void tempsreel_free(void* ptr){
    for(int i = 0; i<4; i++){ 
        if(spaces[i] == ptr){
            libre[i] = 1;
            printf("Free %u\n", i);
            return;
        }
    }
    printf("Cannot free!\n");
}

