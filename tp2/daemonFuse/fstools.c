#include "fstools.h"

struct cacheFichier* incrementeCompteurFichier(const char *path, struct cacheData *cache, int increment){
	struct cacheFichier *fichier = cache->firstFile;

	while(fichier != NULL){
		if(strcmp(fichier->nom, path) == 0){
			fichier->countOpen += increment;
			break;
        }
		fichier = fichier->next;
	}
	return fichier;
}

struct cacheFichier* trouverFichierEnCache(const char *path, struct cacheData *cache){
	return incrementeCompteurFichier(path, cache, 0);
}

void insererFichier(struct cacheFichier *infoFichier, struct cacheData *cache){
    if(cache->firstFile == NULL){
        infoFichier->next = NULL;
    }
    else{
        infoFichier->next = cache->firstFile;
        cache->firstFile->prev = infoFichier;
    }
    cache->firstFile = infoFichier;
}

void retireFichier(struct cacheFichier *infoFichier, struct cacheData *cache){
    free(infoFichier->nom);
    free(infoFichier->data);
    if(cache->firstFile == infoFichier)
        cache->firstFile = infoFichier->next;
    if(infoFichier->prev != NULL)
        infoFichier->prev->next = infoFichier->next;
    if(infoFichier->next != NULL)
        infoFichier->next->prev = infoFichier->prev;
    free(infoFichier);
}
