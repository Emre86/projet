/** Client rexecut*/

#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "protocol.h"

/************************************/
/********** Initalisations **********/
/************************************/






/************************************/
/********** Fonctions ***************/
/************************************/





int main(int argc, char *argv[])
{

	int rex_query;
	int rex_resp;
	t_rexecut query;
	t_rexecut_ack resp;

	if (argc != 2 ){
		printf("Usage: %s nom_executable\n", argv[0]);
		exit(0);
	}
/*
 	char tampon[70];
        getcwd(tampon,70);
        printf(" getcwd = %s \n",tampon);
*/

	/*** Ouverture du tube pour envoyer vers le serveur local ***/
	if ( ( rex_query = open("rexecut1",O_WRONLY)) == -1){
	 	perror("open");
		exit(1);
	}
	
	query.msg_type = REXECUT;
/*	strcpy(query.work_dir,tampon);*/
	printf("°°°° on envoie %s °°°° \n", argv[1]);
	strcpy(query.prog, argv[1]);

	/*** Envoie du message dans le tube ***/	
	if( (write(rex_query,&query,sizeof(t_rexecut))) == -1){
		perror("write");
		exit(1);
	}
	
	close(rex_query);
		
	/*** Ouverture du tube pour recevoir des messages venant du serveur local ***/
	if(( rex_resp = open("rexecut2",O_RDONLY)) == -1){
                perror("open");
                exit(1);
        }
	
	/*** Reception du message dans le tube ***/
	if( read(rex_resp,&resp,sizeof(t_rexecut_ack)) == -1){
		perror("read");
		exit(1);
	}	
	printf("°° Rexecut crée avec pour UID = < %d, %s >  °° \n",resp.uid.pid, resp.uid.host); 
	close(rex_resp);
	
        return(0);
}

