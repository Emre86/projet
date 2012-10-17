#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include "librproc.h"

#include "fonctions.h"

int main(int argc, char **argv){

	t_uid uid;
		
	int p;

	
	

	for( p = 0 ; p < 10; p++){
	        if ( rexecut(&uid,"./exec/mon_sleep") == -1 ){
        	        fprintf(stderr, "Erreur rexecut\n");
                	return EXIT_FAILURE;
	        }
		printf("fin %d \n",p);
	}


    return EXIT_SUCCESS;
}
