#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include "librproc.h"



int main(int argc, char **argv){

	t_uid uid;
 	int status; 
	
	t_uid uid2;



	int p; 

	 /* appel à rexecut */ 

	if ( rexecut(&uid, "./exec/monsleep") == -1 ){
		fprintf(stderr, "Erreur rexecut\n");
		return EXIT_FAILURE;
	}

	if( (p=fork()) == -1){
		perror("fork");exit(1);
	}
	if(p == 0){
		if ( rexecut(&uid2, "./exec/mon_sleep") == -1 ){
			fprintf(stderr, "Erreur rexecut\n");
			return EXIT_FAILURE;
		}
		rwaituid(uid2,&status);
		printf(" FIN FILS\n");
		return EXIT_SUCCESS;
	}
	
	rwait(&status);
	printf(" FIN PERE\n");
	 
 
    return EXIT_SUCCESS;
}

