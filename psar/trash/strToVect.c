#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NB_MAX_ARGS 15
#define ARGS_LEN	128


/******************************************
 * strToVect : transforme la chaine de    *
 * caractères input en tableau de chaines *
 ******************************************/

void strToVect(char *str, char **args){
	char *input, **aux;
	input = malloc( (strlen(str) + 1) * sizeof(char) );
	strcpy(input, str);

	aux = args;
	while ( (*aux = strsep(&input, " \t")) != NULL ){
		aux ++;
	}
}
	
/*
int main(){

	char *res[NB_MAX_ARGS];
	
	strToVect("bond jdd", (char**)&res);

	strcpy(res[1], "123456789");
	
	printf("%s %s\n", res[0], res[1]);

	return EXIT_SUCCESS;
}
*/
