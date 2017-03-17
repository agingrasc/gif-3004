#include "allocateurMemoire.h"
#include <stdint.h>

// Resize : 2*image (3 canaux) + 2*image (1 canal)
// Low-pass : 2*image (3 canaux)
// High-pass : 3*image (3 canaux)

#define ALLOC_N_BIG 16
#define ALLOC_N_SMALL 100
#define ALLOC_SMALL_SIZE 1024

#define MAX_MEMORY_SPACE 12

static void* spaces[MAX_MEMORY_SPACE];
static uint8_t libre[MAX_MEMORY_SPACE];

int prepareMemoire(size_t tailleImageEntree, size_t tailleImageSortie){

    for(int i = 0; i<MAX_MEMORY_SPACE; i++){
        spaces[i] = malloc(4194304);
        libre[i] = 1;
    }

}

void* tempsreel_malloc(size_t taille){
    for(int i = 0; i<4; i++){ 
        if(libre[i] == 1){
            libre[i] = 0;
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
            return;
        }
    }
    printf("Cannot free!\n");
}

