//============================================================================
// Name        : setr_tp1.cpp
// Author      : Marc-Andre Gardner
// Version     : Hiver 2017
// Description : Fichier d'introduction a la compilation croisee et au 
//               debogage distant
//============================================================================

#include <stdlib.h>
#include <stdio.h>

// Definit les modes de tri possibles
#define INIT_ORDONNE  1				// Le tableau est initialise en ordre croissant
#define INIT_INVERSE  2				// Le tableau est initialise en ordre decroissant
#define INIT_ORDSAUF1 3				// Le tableau est initialise en ordre croissant, sauf deux elements qui sont inverses

// Nombre d'elements dans le tableau
#define N_ELEM 2000


int fonctionComparatrice(void const *a, void const *b){
	// Cette fonction doit retourner -1 si a < b,
	// 0 s'ils sont egaux et 1 sur a > b

	// On commence par convertir les arguments d'entree dans le bon type
	const int *cmp1 = (const int *) a;
	const int *cmp2 = (const int *) b;

	// On effectue la comparaison
	if(cmp1 < cmp2)
		return -1;
	else if(cmp1 == cmp2)
		return 0;
	else
		return 1;
}


void maFonctionDeTri(int* base, int num, int size, int (*cmpfunc)(const void*,const void*)){
	// Implementation naive du tri par insertion
	// Reference : https://en.wikipedia.org/wiki/Insertion_sort
	int tmp, j;

	for(int i=0; i<num; i++){
		tmp = base[i];
		j = i;
		while(j > 0 && cmpfunc(base+(j-1), &tmp) > 0){
			base[j] = base[j-1];
			j--;
		}
		base[j] = tmp;
	}
}


void initTableau(int* t, int len, char typeinit){
	// Fonction implementant les differents modes d'initialisation
	// decrits plus haut
	int s;
	switch(typeinit){
	case INIT_ORDONNE:
		for(int i=0; i<len; i++)
			t[i] = i+1;
		break;

	case INIT_INVERSE:
		for(int i=0; i<len; i++)
			t[i] = N_ELEM - i;
		break;

	case INIT_ORDSAUF1:
		for(int i=0; i<len; i++)
			t[i] = i+1;

		s = t[len/2];
		t[len/2] = t[len/2+1];
		t[len/2+1] = s;
		break;

	default:
		printf("Mode d'initialisation %i inconnu!\n", typeinit);
	}
}


int main(int argc, char *argv[]) {
	int numbers[N_ELEM];				// On alloue un tableau d'entiers (int)
	char modeinit = INIT_INVERSE;		// Par defaut, l'initialisation est en ordre decroissant
	if(argc > 1)
		modeinit = (char)atoi(argv[1]); // Si l'utilisateur a indique un mode, on l'utilise

	printf("Initialisation du tableau de nombres (mode %i)...\n", modeinit);
	initTableau(numbers, N_ELEM, modeinit);
	printf("Premier et dernier elements du tableau (pre-tri) : %i / %i\n", numbers[0], numbers[N_ELEM-1]);

	// Utilisation de la fonction qsort de la librairie standard du C
	// Voir ici pour une description de son utilisation : http://www.cplusplus.com/reference/cstdlib/qsort/
	qsort(numbers, 					// Notre tableau de nombres a trier
			N_ELEM, 				// Le nombre d'elements dans le tableau
			8, 						// Le nombre d'octets par element;
									// 	le programme est compile sur un ordinateur 64 bits (celui sur lequel Eclipse est installe), donc un int fait 8 octets
			fonctionComparatrice);  // La fonction permettant d'ordonner deux elements

	printf("Premier et dernier elements du tableau (post-tri, fonction de tri de la librairie standard) : %i / %i\n", numbers[0], numbers[N_ELEM-1]);

	// On recommence avec la fonction de tri personnalisee
	printf("Initialisation du tableau de nombres (mode %i)...\n", modeinit);
	initTableau(numbers, N_ELEM, modeinit);
	printf("Premier et dernier elements du tableau (pre-tri) : %i / %i\n", numbers[0], numbers[N_ELEM-1]);

	// Utilisation d'une fonction de tri personnalisee
	// Sa signature est la meme que qsort, pour faciliter les tests
	maFonctionDeTri(numbers, N_ELEM, 8, fonctionComparatrice);
	printf("Premier et dernier elements du tableau (post-tri, fonction de tri personnalisee) : %i / %i\n", numbers[0], numbers[N_ELEM-1]);

	return 0;
} 
