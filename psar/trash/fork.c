#define _POSIX_SOURCE 1

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


void handler(int sig){
	printf("Processus %d : Signal reçu : %d\n", getpid(), sig);	
}

/*
 * Créer un fils qui execute "sleep 8"
 */

int main(int argc, char **argv){
	
	int i;
	pid_t p;

	/* Redéfinition de signaux */
	sigset_t set;
	struct sigaction action;
	
	sigemptyset(&set);
	action.sa_mask = set;
	action.sa_flags = 0;
	action.sa_handler = handler;

	for ( i = 1; i < 32; i++ ){
		if (i != SIGKILL && i != SIGSTOP){
			if ( sigaction(i, &action, NULL) == -1 ){
				perror("sigaction");
				return EXIT_FAILURE;
			}
		}
	}

	
	/* Créer un fils qui execute "sleep 8" et l'attendre */
	if ( (p = fork()) == 0 ){
		execlp("sleep", "sleep", "8", NULL);
	}else{
		printf("Père: Mon PID est %d, celui de mon fils est %d\n", getpid(), p);
		wait(NULL);
	}

	return EXIT_SUCCESS;

}
