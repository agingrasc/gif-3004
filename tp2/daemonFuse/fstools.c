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

int checkPathExistence(const char* path, cacheData* cache) {

    //si rootDirIndex n'est pas rempli, on assume que oui par defense
    if (cache->rootDirIndex == NULL) {
        return 1;
    }

    char *indexStr = malloc(strlen(cache->rootDirIndex) + 1);
    strcpy(indexStr, cache->rootDirIndex);

    char *nomFichier = strtok(indexStr, "\n");        // On assume des fins de lignes UNIX
    int countInode = 1;
    while (nomFichier != NULL) {
        if (strcmp(nomFichier, path+1) == 0)
        {
            return 1;
        }
        nomFichier = strtok(NULL, "\n");
    }
    free(nomFichier);
    return 0;
}
