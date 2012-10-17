/** Client rexecut*/

#define _POSIX_SOURCE 1

#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>



#include "protocol.h"
#include "rproctypes.h"



/* Tubes d'écriture (p_out) et de lecture (p_in) */

/* char tube_out[STRLEN]; */
/* char tube_in[STRLEN]; */

extern void bzero (void *s, size_t n);
 

/************************************/
/********** Fonctions ***************/
/************************************/


/***********************
 *     REXECUT 1       *
 ***********************/

/* actuellement pas de nom_machine précisée car le SL le choisi tout seul 
 * renvoie -1 si erreur
 *
 */

int rexecut(t_uid *uid, char *exec)
{

	/* messages requete et réponse */
	t_rexecut query;
	t_rexecut_ack resp;

	/* sockets */
	int sock;
	struct sockaddr_in dest;
	/* struct hostent *hp; PAS BESOIN */


	/* Création socket */

	if ( ( sock = socket(AF_INET, SOCK_DGRAM, 0) ) == -1){
	 	perror("socket");
		return -1;
	}

	/* Remplir dest */
	bzero((char *)&dest, sizeof(dest));
	dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	dest.sin_family = AF_INET;
	dest.sin_port = htons(PORT_REX);
	
	
	/* Préparer le message */
	query.msg_type = REXECUT;
	query.emetteur = getpid();
	strcpy(query.prog, exec);
	strcpy(query.machine, "any");

	

	/*printf("°°°° on envoie %s °°°° \n", exec);*/


	/* Envoie du message sur la socket */	
	if ( sendto(sock, &query, sizeof(t_rexecut), 0, (struct sockaddr *)&dest, sizeof(dest) ) == -1 ){
		perror("sendto");
		return -1;
	}
	
	
	/* Reception du message */
	if( recvfrom(sock, &resp, sizeof(t_rexecut_ack), 0, 0, 0) == -1){
		perror("rcvfrom");
		return -1;
	}

	
	printf("°° Rexecut crée avec pour UID = < %d, %s >  °° \n", resp.uid.pid, resp.uid.host); 
    
	uid->pid = resp.uid.pid;
	strcpy(uid->host, resp.uid.host);


	close(sock);
	
	if (uid->pid == -1)
		return -1;
	else
	    return uid->pid; /* ou autre > 0 */
}


/***********************
 *     REXECUT 2       *
 ***********************/

/* Version avec nom_machine précisée car le SL le choisi tout seul 
 * renvoie -1 si erreur
 *
 */

int rexec(t_uid *uid, char *exec, char *machine)
{

	/* messages requete et réponse */
	t_rexecut query;
	t_rexecut_ack resp;

	/* sockets */
	int sock;
	struct sockaddr_in dest;
	/* struct hostent *hp; PAS BESOIN */


	/* Création socket */

	if ( ( sock = socket(AF_INET, SOCK_DGRAM, 0) ) == -1){
	 	perror("socket");
		return -1;
	}

	/* Remplir dest */
	bzero((char *)&dest, sizeof(dest));
	dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	dest.sin_family = AF_INET;
	dest.sin_port = htons(PORT_REX);
	
	
	/* Préparer le message */
	query.msg_type = REXECUT;
	query.emetteur = getpid();
	strcpy(query.prog, exec);
	strcpy(query.machine, machine);

	

	/*printf("°°°° on envoie %s °°°° \n", exec);*/


	/* Envoie du message sur la socket */	
	if ( sendto(sock, &query, sizeof(t_rexecut), 0, (struct sockaddr *)&dest, sizeof(dest) ) == -1 ){
		perror("sendto");
		return -1;
	}
	
	
	/* Reception du message */
	if( recvfrom(sock, &resp, sizeof(t_rexecut_ack), 0, 0, 0) == -1){
		perror("rcvfrom");
		return -1;
	}

	
	printf("°° Rexecut crée avec pour UID = < %d, %s >  °° \n", resp.uid.pid, resp.uid.host); 
    
	uid->pid = resp.uid.pid;
	strcpy(uid->host, resp.uid.host);


	close(sock);
	
	if (uid->pid == -1)
		return -1;
	else
	    return uid->pid; /* ou autre > 0 */
}



/***********************
 *     RKILL           *
 ***********************/

int rkill(t_uid uid, int sig)
{

	/* messages requete et réponse */
	t_rkill query;
	t_rkill_ack resp;

	/* sockets */
	int sock;
	struct sockaddr_in dest;
	/* struct hostent *hp; PAS BESOIN */


	/* Création socket */
	if ( ( sock = socket(AF_INET, SOCK_DGRAM, 0) ) == -1){
	 	perror("socket");
		return -1;
	}

	/* Remplir dest */
	bzero((char *)&dest, sizeof(dest));
	dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	dest.sin_family = AF_INET;
	dest.sin_port = htons(PORT_RKILL);
	
	query.msg_type = RKILL;
	query.uid      = uid ;
	query.signo    = sig ;

	/* Envoie du message sur la socket */	
	if ( sendto(sock, &query, sizeof(t_rkill), 0, (struct sockaddr *)&dest, sizeof(dest) ) == -1 ){
		perror("sendto");
		return -1;
	}
	
	/*printf("Signal envoyé : %d, au processus %d sur %s\n", query.signo, query.uid.pid, query.uid.host);*/

	
	/* Reception du message */
	if( recvfrom(sock, &resp, sizeof(t_rkill_ack), 0, 0, 0) == -1){
		perror("rcvfrom");
		return -1;
	}

	
	/*printf("Message recu %d\n", resp.msg_type);*/
    
	

	close(sock);

	return EXIT_SUCCESS;
	
}


/***********************
 *     RWAIT           *
 ***********************/

int rwait(int *status){

	t_rwait query;
	t_rwait_ack resp;

	/* sockets */
        int sock;
        struct sockaddr_in dest;
        /* struct hostent *hp; PAS BESOIN */


        /* Création socket */

        if ( ( sock = socket(AF_INET, SOCK_DGRAM, 0) ) == -1){
                perror("socket");
                return -1;
        }

        /* Remplir dest */
        bzero((char *)&dest, sizeof(dest));
        dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        dest.sin_family = AF_INET;
        dest.sin_port = htons(PORT_RWAIT);



        query.msg_type = RWAIT;
        query.emetteur = getpid();
		query.uid.pid = -1;

        /* Envoie du message sur la socket */
        if ( sendto(sock, &query, sizeof(t_rwait), 0, (struct sockaddr *)&dest, sizeof(dest) ) == -1 ){
                perror("sendto");
                return -1;
        }
        /*printf("°°°° rwait : %d envoie un rwait °°°° \n",query.emetteur);
	*/
	
	
	 /* Reception du message */
        if( recvfrom(sock, &resp, sizeof(t_rexecut_ack), 0, 0, 0) == -1){
                perror("rcvfrom");
                return -1;
        }


	printf("°°°° rwait : Reception d'un message de type %d , status %d et pid %d °°°° \n", resp.msg_type,WEXITSTATUS(resp.status),resp.uid.pid);
	
	close(sock);

    return 1;
}

/***********************
 *     RWAITUID           *
 ***********************/

int rwaituid(t_uid uid, int *status){

	t_rwait query;
	t_rwait_ack resp;

	/* sockets */
        int sock;
        struct sockaddr_in dest;
        /* struct hostent *hp; PAS BESOIN */


        /* Création socket */

        if ( ( sock = socket(AF_INET, SOCK_DGRAM, 0) ) == -1){
                perror("socket");
                return -1;
        }

        /* Remplir dest */
        bzero((char *)&dest, sizeof(dest));
        dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        dest.sin_family = AF_INET;
        dest.sin_port = htons(PORT_RWAIT);



        query.msg_type = RWAIT;
        query.emetteur = getpid();
	query.uid.pid = uid.pid;
	strcpy(query.uid.host,uid.host);

        /* Envoie du message sur la socket */
        if ( sendto(sock, &query, sizeof(t_rwait), 0, (struct sockaddr *)&dest, sizeof(dest) ) == -1 ){
                perror("sendto");
                return -1;
        }
        /*printf("°°°° rwaituid : %d envoie un rwait °°°° \n",query.emetteur);*/
	
	
	
	 /* Reception du message */
        if( recvfrom(sock, &resp, sizeof(t_rexecut_ack), 0, 0, 0) == -1){
                perror("rcvfrom");
                return -1;
        }


	/*printf("°°°° rwaituid : Reception d'un message de type %d , status %d et pid %d °°°° \n",resp.msg_type,WEXITSTATUS(resp.status),resp.uid.pid);*/

	close(sock);

	if ( resp.uid.pid == -1 ){
		fprintf(stderr, "Pas de processus à attendre\n");
	}


    return resp.uid.pid;
}
