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


	
	
	if ( rexecut(&uid, "sleep 20") == -1 ){
		fprintf(stderr, "Erreur rexecut\n");
		return EXIT_FAILURE;
	}

	


	rkill(uid,SIGSTOP);
	sleep(10);
	rkill(uid,SIGCONT);


    return EXIT_SUCCESS;
}
