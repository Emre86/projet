/*
 * Fichier: fonctions.c
 * Description: diverses fonctions utilitaires
 *
 */
#define POSIX_SOURCE 1

#include <stdlib.h>
#include <string.h>

#include "protocol.h"


/****************************************** 
 * uidcmp : permet de comparer 2 uid      *
 * Renvoie 0 si uid1 = uid2               *
 * c'est à dire si ils désignent le même  *
 * pid sur le même hôte                   *
 ******************************************/
 
int uidcmp(t_uid uid1, t_uid uid2){

	if ( (uid1.pid == uid2.pid) && (strcmp(uid1.host, uid2.host) == 0) )
		return 0;
	else
		return 1;
}







/******************************************
 * strToVect : transforme la chaine de    *
 * caractères str en tableau de chaines   *
 * Destinés à être envoyés sur socket     *
 ******************************************/
char *strsep(char **strp, const char *delim);

void strToVect(char *str, char **args){
	char *input, **aux;
	input = malloc( (strlen(str) + 1) * sizeof(char) );
	strcpy(input, str);
	aux = args;
	while ( (*aux = strsep(&input, " \t")) != NULL ){
		/* strcpy(args[i], *aux); */
		aux ++;
	}
}



/* Exemple d'utilisation : 
int main(){
	int i;
	char *res[NB_MAX_ARGS];
	
	strToVect("j'aime pas le psar", (char**)&res);
	
	for (i = 0

	return EXIT_SUCCESS;
}
*/
