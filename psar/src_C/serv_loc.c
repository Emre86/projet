/* Serveur Local */

#define _POSIX_SOURCE 1

#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "local.h"

#define TAILMSG         80
#define NB_MAX_SERV     10
#define RECU_RPS_NO 	12
#define RECU_RPS_OK	13
#define MAX_RWAIT	20

extern int gethostname(char *nom, size_t lg);

/* NOTE : protection :
	=> Toutes les files : mutex_file
	=> numQuery : mutex_file, car manipulé au moment d'insérer ds la file de requête
	=> suivant : mutex
	=> cond : mutex_cond
	=> tab_proc : mutex

*/


 /* compteur (nbproc) et tableau (tab_proc) des processus */
 /* répartis lancés par ce serveur */

int nbproc = 0;
struct proces tab_proc[NPROC_MAX];


/* Compteur de requêtes */
int numQuery = 0;



/* --------- REXECUT ------------- */

/* Socket de communication avec le client */
int sock_rex;

/* File de requêtes REXECUT */
int next_rex = 0;
int nbRexQuery = 0;
rex_query file_rex[MAXREQ];

/* File de réponses REXECUT */
int next_rex_ack = 0;
int nbRexAck = 0;
t_rexecut_ack file_rex_ack[MAXREQ];

pthread_mutex_t mutex_cond_rex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_rex = PTHREAD_COND_INITIALIZER;

/* -------- FIN REXECUT ----------- */

/* --------- RKILL ------------- */

/* Socket de communication avec le client */
int sock_rkill;

/* File de requêtes RKILL */
int next_rkill = 0;
int nbRkillQuery = 0;
rkill_query file_rkill[MAXREQ];

/* File de réponses RKILL_ACK */
int next_rkill_ack = 0;
int nbRkillAck = 0;
t_rkill_ack file_rkill_ack[MAXREQ];

pthread_mutex_t mutex_cond_rkill = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_rkill = PTHREAD_COND_INITIALIZER;

/* -------- FIN REXECUT ----------- */

/* RWAIT */

/* Socket de communication avec le client */
int sock_rwait;

int nbRwaitQuery = 0;


/* ------- FIN RWAIT -------*/


/* ------ Début ajout ------ */

extern void bcopy (const void *src, void *dest, size_t n);
extern void bzero (void *s, size_t n);

/************************************/
/********** Initalisations **********/
/************************************/

struct sockaddr_in dest;
struct hostent *hp;
int sock_UDP;

struct sockaddr_in tcp[NB_MAX_SERV];
struct hostent *hp_tcp[NB_MAX_SERV];
int sock_TCP[NB_MAX_SERV];

int fromlen = sizeof (struct sockaddr_in);


/******* Definition pour les threads *******/

/* Thread pour l'envoi des demandes RPS au serveur distant */
pthread_t tid_env_rps;
pthread_attr_t attr_env_rps;

/* Thread pour l'envoi des demandes REXECUT au serveur distant */
pthread_t tid_env_rex;
pthread_attr_t attr_env_rex;

/* Thread pour la reception des messages RPS du serveur distant */
pthread_t tid_rcpt_rps;
pthread_attr_t attr_rcpt_rps;

/* Thread pour la reception des messages du serveur distant (sauf RPS)  */
pthread_t tid_recpt_dist[NB_MAX_SERV];
pthread_attr_t attr_recpt_dist[NB_MAX_SERV];

/* Thread pour la communication avec le client */
pthread_t tid_comm_rps;
pthread_attr_t attr_comm_rps;

/* Thread pour la communication avec le client */
pthread_t tid_comm_rex;
pthread_attr_t attr_comm_rex;

/* Thread pour la communication avec le client */
pthread_t tid_comm_rwait;
pthread_attr_t attr_comm_rwait;

/* Thread pour la communication avec le client */
pthread_t tid_comm_recpt_rwait;
pthread_attr_t attr_comm_recpt_rwait;


/* Thread pour la communication avec le client */
pthread_t tid_comm_rkill;
pthread_attr_t attr_comm_rkill;


/* --------------- AJOUTS VARIABLES ------------- */


/* --------- Threads pour la réception des demandes DU CLIENT ---------- */
pthread_t tid_rcpt_rex;
pthread_t tid_rcpt_rkill;



/* -------------- FIN AJOUTS VARIABLES ---------- */


/* MUTEX et CONDITIONS */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_cond = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex_file = PTHREAD_MUTEX_INITIALIZER;

/* attention : si mise en place de mutex dédiés par type de file, 
   protéger également numQuery par un mutex dédié */



/******* Definition des signaux *******/
struct sigaction act;
sigset_t sig;

/* Variable pour le nombre de serveurs disponibles au debut */
int nb_serv_init;

/* Variable pour le nombre de serveurs disponibles */
int nb_serv;

/* Structure pour les serveurs disponible */
typedef struct serveur_dispo
{
  char name[HOST_NAME_MAX_LEN];
  int presence;
} tableau_serveur;

/* Tableau des serveurs avec leur nom et leur presence */
tableau_serveur tab_serv[NB_MAX_SERV];

/* Variable representant le serveur suivant a qui on va envoye un rexecut*/
int suivant;


char *temp;

int nb_rec;

int recu;

/* Mutex pour les rps*/
pthread_mutex_t block_nbrps = PTHREAD_MUTEX_INITIALIZER;

/* Variable pour la terminaison */
int fin;




typedef struct rps_ack_client
{
  int val;
  t_rps_response_client mes;
} rps_ack_cli;

rps_ack_cli reponse_rps_ack;


int handler = 0;


char tmp[HOST_NAME_MAX_LEN + 6] = "cliloc";
char tmp2[HOST_NAME_MAX_LEN + 6] = "loccli";

char mpt[HOST_NAME_MAX_LEN];
char mpt2[HOST_NAME_MAX_LEN];


/************************************/
/********* Fonctions ****************/
/************************************/



/*
 * Fonction de traitement 
 * des signaux
 *
 */
void signo (int sig){

	exit(1);
}
void clean(){
	int i, type;

	
	close(sock_rex);
	close(sock_rwait);	
    
	printf("!!!!!!!!!!!!!!! JE PASSE DANS LE HANDLER !!!!!!!!!!!!!!!!! \n"); 

	unlink (tmp);
	unlink (tmp2);
	printf ("SIG--- Fermeture \n");
	fin = 1;
	for (i = 0; i < nb_serv_init; i++){
		if( tab_serv[i].presence == 1){
			type = FIN;
			printf ("SIG--- Envoie du message FIN %i vers le serveur \n", i);

			/* Envoyer le message */
			if (write (sock_TCP[i], &type, sizeof (int)) == -1){
				printf ("write : sock_TCP[%d]",i);
				exit (1);
			}
			close (sock_TCP[i]);
		}
	}
 
}


/**
 * next_server: détermine le serveur suivant
 **/

int next_server(int pos){
    int suivant, i;
    for (i = 0; i <= nb_serv_init; i++){
	suivant = (pos + i) % nb_serv_init;
	/*printf("Serveur SUIVANT == %d et %s\n",suivant,tab_serv[suivant].name);*/
	if ( ((suivant != pos) & (nb_serv > 1)) | ( nb_serv == 1) ){
		if ( tab_serv[suivant].presence == 1 ){
			/*printf("Serveur SUIVANT CHOISI == %d et %s ::: nb_serv = %d et presence = %d \n",suivant,tab_serv[suivant].name,nb_serv,tab_serv[i].presence);*/
			return suivant;
		}
    	}
    }
    return -1;
}

/**
 * find_server: trouve la position du serveur correspondant au nom, -1 sinon
 **/
int find_server(char *name){
    int i, trouve = -1;
    for (i = 0; i <= nb_serv_init; i++){
		if ( (tab_serv[suivant].presence == 1) && (strcmp(tab_serv[i].name, name) == 0) ){
			trouve = i;
			break;
		}
	}
	return trouve;
}




/**
 ** Fonction pour l'envoi des requetes rkill au serveur distant
 **
 **/

void *envoie_rkill (void *arg){


	return NULL;
}

/**
 ** Fonction pour la reception des requetes rps du serveur distant
 **
 **/

void *recpt_rps (){
	t_rps_response mes_rep;
	int i;
	int j = 0;
	t_rps_response_client cli_rep;

	cli_rep.msg_type = RPS_RESPONSE;

	/*while (1){*/
     		while (nb_rec != nb_serv){

			/* Recevoir la reponse */
			if (recvfrom(sock_UDP, &mes_rep, sizeof (mes_rep), 0, 0,(socklen_t *) & fromlen) == -1){
				perror ("recvfrom");
				exit (1);
			}

			printf("++++++ recpt_rps : CLient_Rps_Recpt : Reponse : %d ++++++\n",mes_rep.msg_type);
			for (i = 0; i < NPROC_MAX; i++){
				if (mes_rep.prog[i].uid.pid != -1001){
		  			printf("++++++ recpt_rps : Client_Rps_Recpt : %d et %d ++++++\n",mes_rep.prog[i].uid.pid,mes_rep.prog[i].etat_processus);
					cli_rep.prog[j]= mes_rep.prog[i];
					j++;
				}
		    	}
			nb_rec++;
		}
		nb_rec = 0;
    		reponse_rps_ack.val = RECU_RPS_OK;
		reponse_rps_ack.mes = cli_rep;
		for (i = 0; i < NPROC_MAX * 10; i++){
			cli_rep.prog[i].uid.pid = PROC_NULL;
			cli_rep.prog[i].etat_processus = ETAT_NULL;
		}

    	/*}*/
	return NULL;
}

/**
 ** Fonction pour l'envoi des requetes rps
 **
 **/

void *envoie_rps ()
{

	t_rps_query mes_req;
	int i;


	for (i = 0; i < nb_serv_init; i++){
		if ( tab_serv[i].presence == 1 ){
		printf("++++++++ envoie_rps : On veut envoyer rps vers =>%s  ++++++++ \n",tab_serv[i].name);

		/* Remplir la structure dest */
			if ((hp = gethostbyname (tab_serv[i].name)) == NULL){
			    perror ("gethostbyaddr erreur ");
			    exit (1);
			}
		
			bzero ((char *) &dest, sizeof (dest));
			bcopy (hp->h_addr_list[0], (char *) &dest.sin_addr, hp->h_length);
			dest.sin_family = AF_INET;
			dest.sin_port = htons (PORTDIST);


			/* Contruire le message ... */
			mes_req.msg_type = RPS_QUERY;

			/* Envoyer le message */
			if (sendto(sock_UDP, &mes_req, sizeof (mes_req), 0, (struct sockaddr *) &dest,sizeof (dest)) == -1){
				perror ("sendto");
				exit (1);
			}
		}
	}
	pthread_cancel ((pthread_t) & tid_env_rps);
	return NULL;
}


/**
 ** Fonction pour la communication avec le client rexecut
 **
 **/

void *client_rexecut ()
{

	int emetteur, id_req, i, sock;
	int reponse = 0;

	t_rexecut query;
	t_rexecut_ack resp;
	struct sockaddr_in expd;
	

    /* Retirer une requête de la file */
	pthread_mutex_lock(&mutex_file);


	query = file_rex[next_rex].query;
	expd = file_rex[next_rex].exp;
	
	/* ajout d'un identifiant à la requête à la requête */
	/* ATTENTION ICI : id_req protégé par mutex_file, et 1 seul mutex pour toutes les files */
	id_req = (++numQuery) % INT_MAX;
	printf("IDREQ : %d\n", id_req);

	query.id_req = id_req;

	next_rex = (next_rex + 1) % MAXREQ;
	nbRexQuery--;
	
	pthread_mutex_unlock(&mutex_file);


	emetteur = query.emetteur;
	printf("++++ client_rexecut %d: Reception d'un message du client de type %d ++++ \n", (int)pthread_self(), query.msg_type);


	/* On lance le Rexecut sur la machine demandée ou le prochain serveur si non précisé */
	pthread_mutex_lock(&mutex);
	if ( strcmp(query.machine, "any") != 0 ){
		
		sock = find_server(query.machine);
		if ( sock != -1 ){

			printf("++++++ envoie_rex (1) : Envoie du message REXECUT %s vers le serveur %d et %s ++++++ \n",query.prog, sock, tab_serv[sock].name);

			/* Envoyer le message */
			if (write (sock_TCP[sock], &query, sizeof (t_rexecut)) == -1){
				perror ("write");
				exit (1);
			}

		}else{
			resp.msg_type = REXECUT_ACK;			
			resp.uid.pid = -1;
			strcpy(resp.uid.host, query.machine);
			resp.id_req = query.id_req;
			

			/* Envoi d'un REXECUT_ACK -1 au client */
			if ( sendto (sock_rex, &resp, sizeof (t_rexecut_ack), 0, (struct sockaddr *)&expd, sizeof(expd) ) == -1){
				perror ("sendto");
				exit (1);
			}

			pthread_mutex_unlock(&mutex);
			pthread_exit((void*)0);

		}
		 



	}	
	else{


		suivant = next_server(suivant);

		printf("++++++ envoie_rex : Envoie du message REXECUT %s vers le serveur %d et %s ++++++ \n",query.prog, suivant, tab_serv[suivant].name);

		/* Envoyer le message */
		if (write (sock_TCP[suivant], &query, sizeof (t_rexecut)) == -1){
			perror ("write");
			exit (1);
		}


	}
	pthread_mutex_unlock(&mutex);

	
	/* Chercher sa réponse */
	while (1){
		
		printf("nbRexAck : %d\n", nbRexAck);		

		pthread_mutex_lock(&mutex_file);
		for (i = 0; i < nbRexAck; i++){
			/*printf("client_rexecut : On cherche une réponse d'id %d\n", id_req);
			printf("client_rexecut : On a trouvé une réponse d'id %d, en position %d\n", file_rex_ack[(next_rex_ack + i) % MAXREQ].id_req, (next_rex_ack + i) % MAXREQ);*/
			if ( file_rex_ack[(next_rex_ack + i) % MAXREQ].id_req == id_req ){
				
				
				reponse = 1;
				resp = file_rex_ack[(next_rex_ack + i) % MAXREQ];

				/*printf("Réponse trouvée : id_req = %d\n", resp.id_req);*/
				
				/* 'supprimer' la réponse de la file */

				nbRexAck--;
				/*printf("Avant : next_rex_ack = %d\n", next_rex_ack);*/
				next_rex_ack = (next_rex_ack + 1) % MAXREQ;
				/*printf("Apres : next_rex_ack = %d\n", next_rex_ack);*/


				break;
			}	
		}
		pthread_mutex_unlock(&mutex_file);
		
		if (!reponse){
			pthread_mutex_lock(&mutex_cond_rex);
			/*printf("Thread %d : Attente sur la condition de réponse\n",  (int)pthread_self());*/
			pthread_cond_wait(&cond_rex, &mutex_cond_rex);
			pthread_mutex_unlock(&mutex_cond_rex);
		}else{
			break;
		}
	}
	/* Réponse connue */
	/*printf("Sortie while : Réponse trouvée : uid = %d\n", resp.uid.pid);*/								

	/* mettre a jour la structure du serveur local */
	pthread_mutex_lock(&mutex);
	nbproc ++;
	tab_proc[nbproc-1].finished = 0;
	tab_proc[nbproc-1].status = 0;
	tab_proc[nbproc-1].uid.pid = resp.uid.pid;
	strcpy(tab_proc[nbproc-1].uid.host, resp.uid.host);	
	tab_proc[nbproc-1].pid_emet = emetteur;

	printf("++++ client_rexexut : nbproc = %d , pid_processus = %d  et emetteur = %d ++++++\n",nbproc,tab_proc[nbproc-1].uid.pid,tab_proc[nbproc-1].pid_emet);
		
	pthread_mutex_unlock(&mutex);
	

	/* Envoi des données au client */
	if ( sendto (sock_rex, &resp, sizeof (t_rexecut_ack), 0, (struct sockaddr *)&expd, sizeof(expd) ) == -1){
		perror ("sendto");
		exit (1);
	}

	

	/*printf("++++ client_rexecut : Envoie d'un message vers le client de type %d ++++ \n", resp.msg_type);*/

	/*	printf("Thread %d : je me termine\n", (int)pthread_self());*/
    

	pthread_exit((void*)0);

} 


/**
 ** Fonction pour la communication avec le client rkill
 **
 **/

void *client_rkill ()
{


	int id_req, i;
	int reponse = 0;

	t_rkill query;
	t_rkill_ack resp;
	struct sockaddr_in expd;
	

    /* Retirer une requête de la file */
	pthread_mutex_lock(&mutex_file);

	/*printf("++++ client_rkill, next_rkill : %d\n", next_rkill);*/

	query = file_rkill[next_rkill].query;
	expd = file_rkill[next_rkill].exp;
	
	/* ajout d'un identifiant à la requête à la requête */

	id_req = (++numQuery) % INT_MAX;
	printf("IDREQ : %d\n", id_req);

	query.id_req = id_req;

	next_rkill = (next_rkill + 1) % MAXREQ;
	nbRkillQuery--;
	
	pthread_mutex_unlock(&mutex_file);


	/*printf("++++ client_rkill %d: Extraction d'un message du client de type %d ++++ \n", (int)pthread_self(), query.msg_type);*/


	/* On lance le rkill sur le prochain serveur */
   	pthread_mutex_lock(&mutex);

	/*suivant = next_server(suivant);*/
	for( i = 0 ; i < nb_serv_init ; i++){
		if ( strcmp(query.uid.host,tab_serv[i].name) == 0){ 
			printf("++++ client_rkill : Envoie du message %d vers le serveur %s, de nom %s ++++++ \n", query.msg_type, query.uid.host, tab_serv[i].name);

			/* Envoyer le message */
			if (write (sock_TCP[suivant], &query, sizeof (t_rkill)) == -1){
				perror ("write");
				exit (1);
	    		}
			break;
		}
	}
	pthread_mutex_unlock(&mutex);


	/* Chercher sa réponse */
	while (1){
		


		pthread_mutex_lock(&mutex_file);
		for (i = 0; i < nbRkillAck; i++){

			if ( file_rkill_ack[(next_rkill_ack + i) % MAXREQ].id_req == id_req ){
				
				
				reponse = 1;
				resp = file_rkill_ack[(next_rkill_ack + i) % MAXREQ];

				
				/* 'supprimer' la réponse de la file */

				nbRkillAck--;

				next_rkill_ack = (next_rkill_ack + 1) % MAXREQ;

				break;
			}	
		}
		pthread_mutex_unlock(&mutex_file);
		
		if (!reponse){
			pthread_mutex_lock(&mutex_cond_rkill);
			/*printf("Thread %d (rkill): Attente sur la condition de réponse\n",  (int)pthread_self());*/
			pthread_cond_wait(&cond_rkill, &mutex_cond_rkill);
			pthread_mutex_unlock(&mutex_cond_rkill);
		}else{
			break;
		}
	}
	/* Réponse connue */
	printf("Sortie while : Réponse trouvée : id_req = %d\n", resp.id_req);								



	/* Envoi des données au client */
	if ( sendto (sock_rex, &resp, sizeof (t_rkill_ack), 0, (struct sockaddr *)&expd, sizeof(expd) ) == -1){
		perror ("sendto");
		exit (1);
	}
    

  pthread_exit((void*)0);

    return NULL;
}



/**
 ** Fonction pour la communication avec le client rps
 **
 **/

void *client_rps ()
{


  int query;
  int resp;
  int n;
  int i;
  t_rps_query que;
  t_rps_response res;

	/*** Creation du tube venant du client ***/
	if (mkfifo (tmp, S_IRUSR | S_IWUSR) == -1){
		perror ("mkfifo : cliloc");
		exit (1);
	}

	/*** Creation du tube vers le client ***/
	if (mkfifo (tmp2, S_IRUSR | S_IWUSR) == -1){
		perror ("mkfifo : loccli");
		exit (1);
	}
	while (1){
		/*** Ouverture du tube venant du client ***/
		if ((query = open (tmp, O_RDONLY)) == -1){
			perror ("open: cliloc");
			exit (2);
		}

		/*** Ouverture du tube vers le client ***/
		if ((resp = open (tmp2, O_WRONLY)) == -1){
			perror ("open : loccli");	
			exit (2);
		}

		/* lit les requete, bloque si rien dans le tube */
		if ((n = read (query, &que, sizeof (t_rps_query))) < 0){
			perror ("read");
			exit (2);
		}
		close (query);
		/*printf("++++ client_rps : Reception d'un message du client de type %d ++++ \n",que.msg_type);*/


		/*** Reception des messages Election ***/
		/*if (pthread_attr_init (&attr_env_rps) != 0){
			perror ("pthread_attr_init");
			exit (1);
		}
		if (pthread_create (&tid_env_rps, &attr_env_rps, envoie_rps, NULL) != 0){
			perror ("pthread_create");
			exit (1);
		}


		while (reponse_rps_ack.val != RECU_RPS_OK){
		}
		*/
		envoie_rps();
		recpt_rps();

		/*** Ecriture dans le tube vers le client ***/
		if ((write (resp, &reponse_rps_ack.mes, sizeof (t_rps_response_client))) == -1){
			perror ("write");
			exit (1);
		}

		printf("++++ client_rps : Envoie d'un message vers le client de type %d ++++ \n", res.msg_type);

      		close (resp);
		reponse_rps_ack.val = RECU_RPS_NO;


		for (i = 0; i < NPROC_MAX * 10; i++){
			reponse_rps_ack.mes.prog[i].uid.pid = PROC_NULL;
			reponse_rps_ack.mes.prog[i].etat_processus = ETAT_NULL;
			strcpy(reponse_rps_ack.mes.prog[i].prog,"NULL");
		}
    	}
	return NULL;
}




/**
 ** Fonction pour la communication avec le client rwait
 **
 **/

void *client_rwait(void * arg){
    

	int i,j;
/*	int query;
	int resp;
	int n;
*/	int fin_rwait = 0;
	int fin_rwaituid = 0;
	int trouve = 0;
	t_rwait_ack res;
	t_rwait_fin mes_fin_rwait;
	struct sockaddr_in expd;

	/*** on doit recupere la position trouver auparavant ***/
	t_rwait_int que = *(t_rwait_int *) arg;
	
	expd = que.exp;


	/*printf("client_rwait : Réception d'un message du client %d de type %d ++++ \n", que.emetteur, que.msg_type);*/
    	
	/* pas de wait pid */
	if ( que.uid.pid == -1){

		while(1){
			pthread_mutex_lock(&mutex);
			for ( i=0;i<nbproc;i++){
				if ( tab_proc[i].pid_emet == que.emetteur){
					if( tab_proc[i].finished == 1){
						fin_rwait = 1;
						trouve = 1;
						break;
					}
					if( tab_proc[i].pid_emet > 0){
						/*printf(" %d et %d;\n",tab_proc[i].pid_emet,tab_proc[i].finished);*/
					}
					trouve = 1;
				}
				
			}
			pthread_mutex_unlock(&mutex);
			printf("++++ client_rwait : parcours du tableau effectue trouve = %d et fin_rwait = %d et pid_proc = %d et que.emetteur = %d ++++\n",trouve,fin_rwait,tab_proc[i].uid.pid,que.emetteur);
			if ( trouve == 0){
        	               	printf(" pas de wait trouve dans le tableau \n");
				res.msg_type = RWAIT_ACK;
	                        res.uid.pid = -1;
        	       	        res.status = 0;
        			
				/* Envoi des données au client */
				if ( sendto (sock_rwait, &res, sizeof (t_rwait_ack), 0, (struct sockaddr *)&expd, sizeof(expd) ) == -1){
					perror ("sendto");
					exit (1);
				}	       	        
	
				/*printf("++++ client_rwait : Envoie d'un message vers le client de type %d ++++ \n",res.msg_type);*/
				break;
			}
			if ( fin_rwait == 1){

				pthread_mutex_lock(&mutex);

				/* envoyer message vers le client avec status */
	                        res.msg_type = RWAIT_ACK;
                                res.uid.pid = tab_proc[i].uid.pid;
                                strcpy(res.uid.host,tab_proc[i].uid.host);
                                res.status = tab_proc[i].status;

                                /* Envoi des données au client */
                                if ( sendto (sock_rwait, &res, sizeof (t_rwait_ack), 0, (struct sockaddr *)&expd, sizeof(expd) ) == -1){
                                        perror ("sendto");
                                        exit (1);
                                }
				for ( j = 0; j< nb_serv;j++){
					if ( strcmp(tab_serv[j].name,tab_proc[i].uid.host) == 0 ){
						mes_fin_rwait.msg_type = RWAIT_FIN;
						mes_fin_rwait.uid = res.uid;
						if (write (sock_TCP[j], &mes_fin_rwait, sizeof (t_rwait_fin)) == -1){
							perror ("write");
							exit (1);
						}
						printf("++++ client_rwait : Envoie realise vers %s , pid = %d  \n",tab_serv[j].name,res.uid.pid);
						break;
					}
					printf(" %s et %s \n",res.uid.host,tab_serv[j].name);

				}

				/*printf("++++ client_rwait : Envoie d'un message vers le client de type %d ++++ \n",res.msg_type);*/
                                tab_proc[i].uid.pid = tab_proc[nbproc-1].uid.pid;
                                tab_proc[i].pid_emet = tab_proc[nbproc-1].pid_emet;
                                tab_proc[i].finished = tab_proc[nbproc-1].finished;
                                strcpy(tab_proc[i].uid.host,tab_proc[nbproc-1].uid.host);
                                tab_proc[i].status = tab_proc[nbproc-1].status;
                                nbproc --;

				pthread_mutex_unlock(&mutex);
					
				break;
			}
			pthread_mutex_lock(&mutex_cond);
			pthread_cond_wait(&cond,&mutex_cond);
			pthread_mutex_unlock(&mutex_cond);
			printf("++++ client_rwait : reveiller ++++\n");
			trouve = 0;
			fin_rwait = 0;
		}

    	}else{
		/* cas du waituid */
		while(1){
			pthread_mutex_lock(&mutex);
			for ( i=0;i<nbproc;i++){
        	        	if ( ( tab_proc[i].pid_emet == que.emetteur) && ( tab_proc[i].uid.pid == que.uid.pid) && ( strcmp(tab_proc[i].uid.host,que.uid.host) == 0 ) ) {
                	        	if ( tab_proc[i].finished == 1){
        	                           	fin_rwaituid = 1;
						trouve = 1;
						break;
					}
					trouve = 1;
                        	}
                        }
			pthread_mutex_unlock(&mutex);
	                if ( fin_rwaituid == 1){
				pthread_mutex_lock(&mutex);

				res.msg_type = RWAIT_ACK;
                                res.uid.pid = tab_proc[i].uid.pid;
                                strcpy(res.uid.host,tab_proc[i].uid.host);
                                res.status = tab_proc[i].status;
                                
				/* Envoi des données au client */
                                if ( sendto (sock_rwait, &res, sizeof (t_rwait_ack), 0, (struct sockaddr *)&expd, sizeof(expd) ) == -1){
                                        perror ("sendto");
                                        exit (1);
                                }

				 for ( j = 0; j< nb_serv;j++){
                                        /*if ( strcmp(tab_serv[j].name,res.uid.host) == 0 ){*/
                                                mes_fin_rwait.msg_type = RWAIT_FIN;
                                                mes_fin_rwait.uid = res.uid;
                                                if (write (sock_TCP[j], &mes_fin_rwait, sizeof (t_rwait_fin)) == -1){
                                                        perror ("write");
                                                        exit (1);
                                                }
                                                printf("++++ client_rwait : Envoie realise vers %s \n",tab_serv[j].name);
                                                break;
                                        /*}*/

                                }

				/*printf("++++ client_rwait : Envoie d'un message vers le client de type %d ++++ \n",res.msg_type);*/
                                tab_proc[i].uid.pid = tab_proc[nbproc-1].uid.pid;
                                tab_proc[i].pid_emet = tab_proc[nbproc-1].pid_emet;
                                tab_proc[i].finished = tab_proc[nbproc-1].finished;
                                strcpy(tab_proc[i].uid.host,tab_proc[nbproc-1].uid.host);
	                        tab_proc[i].status = tab_proc[nbproc-1].status;
				nbproc --;
				pthread_mutex_unlock(&mutex);

				break;
			}
			if ( trouve == 0){
	                	/* envoyer message vers le client avec status */
        	                printf(" pas de rwaituid trouve dans le tableau \n");
        	                res.msg_type = RWAIT_ACK;
                		res.uid.pid = -1;
	                        res.status = 0;
	                        
				/* Envoi des données au client */
                                if ( sendto (sock_rwait, &res, sizeof (t_rwait_ack), 0, (struct sockaddr *)&expd, sizeof(expd) ) == -1){
                                        perror ("sendto");
                                        exit (1);
                                }
				/*printf("++++ client_rwait : Envoie d'un message vers le client de type %d ++++ \n",res.msg_type);*/
				break;
                        }
				
			pthread_mutex_lock(&mutex_cond);
			pthread_cond_wait(&cond,&mutex_cond);
			pthread_mutex_unlock(&mutex_cond);
			trouve = 0;
			fin_rwaituid = 0;
		}
	}
/*	pthread_exit(&tid_comm_rwait);*/
	return NULL;
}

		



/**
 ** Fonction reception_dist
 ** pour la reception des requetes venant du serveur
 ** distant
 **
 **/

void *reception_dist (void *arg){

	int type;
	t_uid uid;
	int status;
	int i;
	int nb_lu;
	t_rexecut_ack resp;
	t_rkill_ack resp_kill;


	/*** on doit recuperer la position trouver auparavant ***/
	int position = *(int *) arg;
	while (nb_serv != 0){

		/*** Attente d'un message ***/
		if ( (nb_lu = read (sock_TCP[position], &type, sizeof (int))) == -1){
			perror ("read");
			exit (1);
		}

		if ( nb_lu == 0){
			/*printf("/////////////////////////////////////\n");*/
			pthread_cancel ((pthread_t) & tid_recpt_dist);
			return NULL;	
		}
		/*** Traitement des messages ***/
		if (type == REXECUT_ACK){
			resp.msg_type = REXECUT_ACK;

			if (read (sock_TCP[position], &resp.uid, sizeof (t_uid)) == -1){
				perror ("read");
				exit (1);
			}
			if (read (sock_TCP[position], &resp.id_req, sizeof (int)) == -1){
				perror ("read");
				exit (1);
			}
			printf("++++++ reception_dist : message REXECUT_ACK ; UID = < %d, %s >  \n", resp.uid.pid, resp.uid.host);


			pthread_mutex_lock(&mutex_file);
			/* dormir 1 sec while nbRexAck >= MAXREQ */
			/* Pour le principe de pas déborder la file,
				mais bon, endormir une thread qui est commune pour toutes les réceptions, 
				c'est moyen quoi */
			while ( nbRexAck >= MAXREQ ){ 
				pthread_mutex_unlock(&mutex_file);
				sleep(1); 
				pthread_mutex_lock(&mutex_file);
			}

			file_rex_ack[next_rex_ack + nbRexAck] = resp;
			
			/* vérifier insertion */
			printf("++++++ reception_dist : insertion du type %d, PID : %d, id_req : %d\n", file_rex_ack[next_rex_ack + nbRexAck].msg_type, file_rex_ack[next_rex_ack + nbRexAck].uid.pid, file_rex_ack[next_rex_ack + nbRexAck].id_req);

			nbRexAck++;
			pthread_mutex_unlock(&mutex_file);

			/*printf("reception_dist : nbRexAck : %d\n", nbRexAck);		*/

			pthread_mutex_lock(&mutex_cond_rex);
			pthread_cond_broadcast(&cond_rex);
			pthread_mutex_unlock(&mutex_cond_rex);


		}else{
			if( type == REXIT){
				/* msg_type, cf ci-dessus */
			
	  			if (read (sock_TCP[position], &uid, sizeof (t_uid)) == -1){	
					perror("read");
					exit(1);
				}
			  	
				if (read (sock_TCP[position], &status, sizeof (int)) == -1){	
					perror("read");
					exit(1);
				}
		
				pthread_mutex_lock(&mutex);
				for ( i=0 ; i< nbproc ; i++){
					if( ( tab_proc[i].uid.pid == uid.pid ) && ( strcmp(tab_proc[i].uid.host,uid.host) == 0 ) ){
						tab_proc[i].finished = 1;
						tab_proc[i].status = status;			
						break;
					}
				}
				
				pthread_mutex_unlock(&mutex);
				printf("++++++ reception_dist : message REXIT traite de pid = %d et emetteur %d ++++++\n",uid.pid,tab_proc[i].pid_emet);
	
				pthread_mutex_lock(&mutex_cond);
				pthread_cond_broadcast(&cond);
				pthread_mutex_unlock(&mutex_cond);

			}else{
				if (type == RKILL_ACK){

					resp_kill.msg_type = RKILL_ACK;

					if (read (sock_TCP[position], &resp_kill.val_ret, sizeof (int)) == -1){
						perror ("read");
						exit (1);
					}
					if (read (sock_TCP[position], &resp_kill.id_req, sizeof (int)) == -1){
						perror ("read");
						exit (1);
					}
					printf("----- reception_dist : RKILL_ACK; requete ID %d, val_ret = %d\n", resp_kill.id_req, resp_kill.val_ret);

					pthread_mutex_lock(&mutex_file);
					/* dormir 1 sec while nbRkillAck >= MAXREQ */
					/* Pour le principe de pas déborder la file,
						mais bon, endormir une thread qui est commune pour toutes les réceptions, 
						c'est moyen quoi */
					while ( nbRkillAck >= MAXREQ ){ 
						pthread_mutex_unlock(&mutex_file);
						sleep(1);
						pthread_mutex_lock(&mutex_file);
					}

					file_rkill_ack[next_rkill_ack + nbRkillAck] = resp_kill;
			

					nbRkillAck++;
					pthread_mutex_unlock(&mutex_file);

					/*printf("reception_dist : nbRkillAck : %d\n", nbRkillAck);*/		

					pthread_mutex_lock(&mutex_cond_rkill);
					pthread_cond_broadcast(&cond_rkill);
					pthread_mutex_unlock(&mutex_cond_rkill);

				}
		  		if (type == FIN){
					
					pthread_mutex_lock(&mutex);
					
					tab_serv[position].presence = 0;
					/* renvoyer des rexecuts pour proc pas fini*/
					
					nb_serv--;
					printf("++++++ reception_dist : FIN du serveur %s presence %d ++++++ \n", tab_serv[position].name, tab_serv[position].presence);
					pthread_mutex_unlock(&mutex);
		    		}
				if(type == TAB_CLEAN){

					if (read (sock_TCP[position], &uid, sizeof (t_uid)) == -1){
                                                perror("read");
                                                exit(1);
                                        }

                                        if (read (sock_TCP[position], &status, sizeof (int)) == -1){
                                                perror("read");
                                                exit(1);
	                                }
        		                printf("++++++ reception_dist : message TAB_CLEAN traite de pid = %d et emetteur %d ++++++\n",uid.pid,tab_proc[i].pid_emet);
				
					pthread_mutex_lock(&mutex);					
					for ( i = 0; i<nbproc ; i++ ){
					
						if ( (tab_proc[i].uid.pid == uid.pid) && ( strcmp(tab_proc[i].uid.host,uid.host) == 0 ) ) {

							tab_proc[i].uid.pid = tab_proc[nbproc-1].uid.pid;
		                                	tab_proc[i].pid_emet = tab_proc[nbproc-1].pid_emet;
        			                	tab_proc[i].finished = tab_proc[nbproc-1].finished;
                				        strcpy(tab_proc[i].uid.host,tab_proc[nbproc-1].uid.host);
        	                		       	tab_proc[i].status = tab_proc[nbproc-1].status;
	                        	        	nbproc --;
							printf("++++++ reception_dist : suppression processus %d ++++++\n",i);
						}
					}	
					pthread_mutex_unlock(&mutex);					
								
				}
			}
		}
    	}
	pthread_cancel ((pthread_t) & tid_recpt_dist);
	return NULL;
}







void *recpt_rwait (){
	
	/* Variable du message */
	t_rwait query;
        t_rwait_ack resp;
	
	/* t_rexecut_ack resp; */
	t_rwait_int tmp;
	t_rwait_int *pt;
	
	int opt = 1;

	/* Variables socket */
	struct sockaddr_in sin;
	struct sockaddr_in expd;
	int fromlen = sizeof(expd);

	/* Création socket */
	if ( ( sock_rwait = socket(AF_INET, SOCK_DGRAM, 0) ) == -1){
	 	perror("socket : recpt_rwait");
		exit(1);
	}

	/* Remplir la socket serveur */
	bzero((char *)&dest, sizeof(dest));
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* INADDR_LOOPBACK */
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT_RWAIT);
	

	if (setsockopt (sock_rwait, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (int)) == -1){
        	perror ("setsockopt");
	        exit (1);
    	}


	/* nommage */
	if ( bind(sock_rwait, (struct sockaddr *)&sin, sizeof(sin)) < 0 ){
		perror("bind : recpt_rwait");
		exit(1);
	}

	
	while(1){
		printf("++ recpt_rwait : DEBUT RECVFROM DU CLIENT\n");
		/* Reception du message */
		if( recvfrom(sock_rwait, &query, sizeof(t_rwait), 0, (struct sockaddr *)&expd, (socklen_t *)&fromlen) == -1){
			perror("recvfrom : recpt_rwait");
			exit(1);
		}

		printf("++ recpt_rwait :  reception rwait du client. TYPE = %d\n", query.msg_type);

		if ( nbRwaitQuery >= MAX_RWAIT ){
			fprintf(stderr, "Nombre max de rwait atteint\n");

			/* retourner t_rwait_ack avec -1 au client */
			resp.msg_type = RWAIT_ACK;
                        resp.uid.pid = -1;
			
                        
                        if ( sendto (sock_rwait, &resp, sizeof (t_rwait_ack), 0, (struct sockaddr *)&expd, sizeof(expd) ) == -1){
                                perror ("sendto");
                                exit (1);
                        }
		

		}else{
			/*** Thread pour la communication avec le client ***/
		        if (pthread_attr_init (&attr_comm_rwait) != 0){
        	        	perror ("pthread_attr_init");
	        	        exit (1);
	        	}

			tmp.exp = expd;
			tmp.emetteur = query.emetteur;
			tmp.msg_type = query.msg_type;
			tmp.uid.pid = query.uid.pid;
			strcpy( tmp.uid.host,query.uid.host);


			pt = malloc(sizeof(t_rwait_int));
			*pt = tmp; 	

		        if (pthread_create (&tid_comm_rwait, &attr_comm_rwait, client_rwait,pt) != 0){
        		        perror ("pthread_create");
                		exit (1);
		        }
			
		}
	}
	return NULL;
}






/* --------------- AJOUTS THREADS ------------- */

/**
 * recpt_rex : Recoit les requetes REXECUT du client local
 * Insère ces requêtes dans la file de requêtes,
 * Crée une thread client_rexecut
 **/

void *recpt_rex (){
	
	/* Variable du message */
	t_rexecut query;
	t_rexecut_ack resp;

	/* Variables socket */
	struct sockaddr_in sin;
	struct sockaddr_in expd;
	int fromlen = sizeof(expd);
	int opt = 1;

	/* Création socket */
	if ( ( sock_rex = socket(AF_INET, SOCK_DGRAM, 0) ) == -1){
	 	perror("socket");
		exit(1);
	}

	/* Remplir la socket serveur */
	bzero((char *)&dest, sizeof(dest));
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT_REX);
	

    if (setsockopt (sock_rex, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (int)) == -1){
        perror ("setsockopt");
        exit (1);
    }


	/* nommage */
	if ( bind(sock_rex, (struct sockaddr *)&sin, sizeof(sin)) < 0 ){
		perror("bind");
		exit(1);
	}

	
	while(1){

		/* Reception du message */
		if( recvfrom(sock_rex, &query, sizeof(t_rexecut), 0, (struct sockaddr *)&expd, (socklen_t *)&fromlen) == -1){
			perror("rcvfrom");
			exit(1);
		}

		printf("++ Recpt_rex : recue rexecut du client de pid %d. TYPE = %d\n", query.emetteur, query.msg_type);

		if ( nbRexQuery >= MAXREQ ){
			fprintf(stderr, "Nombre max de requêtes rexecut atteint\n");
		
			/* retourner t_rexecut_ack avec -1 au client */
			resp.msg_type = REXECUT_ACK;
			resp.uid.pid = -1;
			
			if ( sendto (sock_rex, &resp, sizeof (t_rexecut_ack), 0, (struct sockaddr *)&expd, sizeof(expd) ) == -1){
				perror ("sendto");
				exit (1);
			}

		}else{
			pthread_mutex_lock(&mutex_file);

			file_rex[next_rex + nbRexQuery].query = query;
			file_rex[next_rex + nbRexQuery].exp = expd;

			nbRexQuery++;

			pthread_mutex_unlock(&mutex_file);

			/* Création de thread */
			if (pthread_create (&tid_comm_rex, NULL, client_rexecut, NULL) != 0){
				perror ("pthread_create");
				exit (1);
			}

		}

	}

	return NULL;

}

/**
 * recpt_rkill : Recoit les requetes RKILL du client local
 * Insère ces requêtes dans la file de requêtes,
 * Crée une thread client_rkill
 **/

void *recpt_rkill (){
	
	/* Variable du message */
	t_rkill query;
	t_rkill_ack resp;

	/* Variables socket */
	struct sockaddr_in sin;
	struct sockaddr_in expd;
	int fromlen = sizeof(expd);
	int opt = 1;

	/* Création socket */
	if ( ( sock_rkill = socket(AF_INET, SOCK_DGRAM, 0) ) == -1){
	 	perror("socket");
		exit(1);
	}

	/* Remplir la socket serveur */
	bzero((char *)&dest, sizeof(dest));
	sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT_RKILL);
	

    if (setsockopt (sock_rkill, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (int)) == -1){
        perror ("setsockopt");
        exit (1);
    }


	/* nommage */
	if ( bind(sock_rkill, (struct sockaddr *)&sin, sizeof(sin)) < 0 ){
		perror("bind");
		exit(1);
	}

	
	while(1){

		/* Reception du message */
		if( recvfrom(sock_rkill, &query, sizeof(t_rkill), 0, (struct sockaddr *)&expd, (socklen_t *)&fromlen) == -1){
			perror("rcvfrom");
			exit(1);
		}

		printf("++ recpt_rkill : type : %d, signal %d pour processus %d sur %s\n", query.msg_type, query.signo, query.uid.pid, query.uid.host);

		if ( nbRexQuery >= MAXREQ ){
			fprintf(stderr, "Nombre max de requêtes rkill atteint\n");

			/* retourner t_rkill_ack avec -1 au client */
			resp.msg_type = RKILL_ACK;
			resp.val_ret = -1;

			if ( sendto (sock_rkill, &resp, sizeof (t_rkill_ack), 0, (struct sockaddr *)&expd, sizeof(expd) ) == -1){
				perror ("sendto");
				exit (1);
			}

		}else{
			pthread_mutex_lock(&mutex_file);

			file_rkill[next_rkill + nbRkillQuery].query = query;
			file_rkill[next_rkill + nbRkillQuery].exp = expd;

		/* DEBUG */ /* validé => peut être supprimé */
		printf("++ recpt_rkill : Insertion: type : %d, signal %d pour processus %d sur %s\n", file_rkill[next_rkill + nbRkillQuery].query.msg_type, file_rkill[next_rkill + nbRkillQuery].query.signo, file_rkill[next_rkill + nbRkillQuery].query.uid.pid, file_rkill[next_rkill + nbRkillQuery].query.uid.host);
		/* FIN DEBUG */

			nbRkillQuery++;

			pthread_mutex_unlock(&mutex_file);

			/* Création de thread */
			if (pthread_create (&tid_comm_rkill, NULL, client_rkill, NULL) != 0){
				perror ("pthread_create");
				exit (1);
			}

		}

	}

	return NULL;

}


/* -------------- FIN AJOUTS THREADS ---------- */



/**
 ** main : Programme principal
 **
 **/

int main (int argc, char *argv[]){
	
	int i;
	int *p;
	char temp[HOST_NAME_MAX_LEN];
	
	/* Vérification des arguments */
	if (argc < 2){
		printf ("Usage : %s nom_serveur+ \n", argv[0]);
		return EXIT_FAILURE;
	}

	gethostname(mpt,HOST_NAME_MAX_LEN);
	strcat(tmp,mpt);

	gethostname(mpt2,HOST_NAME_MAX_LEN);
	strcat(tmp2,mpt2);


	/* Traitement des signaux */
	sigfillset (&sig);
	sigdelset (&sig, SIGINT);
	act.sa_handler = signo;
	sigaction (SIGINT, &act, NULL);
	sigprocmask (SIG_SETMASK, &sig, NULL);

	atexit((void *)&clean);

	/* Initialisation des variables */
	nb_rec = 0;
	nb_serv = argc - 1;
	nb_serv_init = nb_serv;
	recu = RECU_RPS_NO;
	fin = 0;
	suivant = 0;




	/* Récupération des noms de serveurs */
	for (i = 0; i < nb_serv; i++){
	    	if ( strcmp(argv[i + 1 ],"localhost") == 0){
	                gethostname(temp,HOST_NAME_MAX_LEN);
			sscanf (temp,"%s", tab_serv[i].name);
		}else{
			sscanf (argv[i + 1], "%s", tab_serv[i].name);
    		strcat(tab_serv[i].name,".infop6.jussieu.fr");
			tab_serv[i].presence = 0;
		}
	}

	/* Création d'une socket non connectée, pour les rps */
	if ( (sock_UDP = socket (AF_INET, SOCK_DGRAM, 0)) == -1 ){
		perror ("socket");
		exit (1);
	}


	for (i = 0; i < nb_serv; i++){
		/* Création de sockets connectées */
		if ( (sock_TCP[i] = socket (AF_INET, SOCK_STREAM, 0)) == -1){
			perror ("socket");
			exit (1);
		}

		/* Résolution du nom */
		printf("Serveur testé : %s\n", tab_serv[i].name);

		if ( (hp_tcp[i] = gethostbyname(tab_serv[i].name)) == NULL ){
			perror ("gethostbyname");
			exit (1);
		}

		printf ("++++ Main : Reussite du Gesthostbyname : %s ++++ \n",
		tab_serv[i].name);

		/* Remplir la structure dest */
		bzero ((char *) &tcp[i], sizeof (tcp[i]));
		bcopy (hp_tcp[i]->h_addr_list[0], (char *) &tcp[i].sin_addr,
		hp_tcp[i]->h_length);
		tcp[i].sin_family = AF_INET;
		tcp[i].sin_port = htons (PORTDIST);

		/* Etablissement de la connexion */
		if ( connect (sock_TCP[i], (struct sockaddr *) &tcp[i], sizeof (tcp[i])) == -1){
			perror ("connect");
			exit (1);
		}

		tab_serv[i].presence = 1;


		/* Thread reception du serveur distant */
		if (pthread_attr_init (&attr_recpt_dist[i]) != 0){
			perror ("pthread_attr_init");
			exit (1);
		}

		p = malloc (sizeof (int));
		*p = i;

		if (pthread_create(&tid_recpt_dist[i], &attr_recpt_dist[i], reception_dist, p) != 0){
			perror ("pthread_create");
			exit (1);
		}

	} /* End For */


	/* ----------------- AJOUTS MAIN ------------------- */

	if (pthread_create (&tid_rcpt_rex, NULL, recpt_rex, NULL) != 0){
		perror ("pthread_create");
		exit (1);
	}

	if (pthread_create (&tid_rcpt_rkill, NULL, recpt_rkill, NULL) != 0){
		perror ("pthread_create");
		exit (1);
	}



	/* --------------- FIN AJOUTS MAIN ----------------- */



	/* Thread pour la reception des messages RPS du serveur distant */
/*	if (pthread_attr_init (&attr_rcpt_rps) != 0){
		perror ("pthread_attr_init");
		exit (1);
	}

	if (pthread_create (&tid_rcpt_rps, &attr_rcpt_rps, recpt_rps, NULL) != 0){
		perror ("pthread_create");
		exit (1);
	}
*/
	/*** RPS : Thread pour la communication avec le client ***/
	if (pthread_attr_init (&attr_comm_rps) != 0){
		perror ("pthread_attr_init");
		exit (1);
	}
	
	if (pthread_create (&tid_comm_rps, &attr_comm_rps, client_rps, NULL) != 0){
		perror ("pthread_create");
		exit (1);
	}


	/*** RWAIT : Thread pour la communication avec le client ***/
	if (pthread_attr_init (&attr_comm_recpt_rwait) != 0) {
		perror ("pthread_attr_init");
		exit (1);
	}
	if (pthread_create (&tid_comm_recpt_rwait, &attr_comm_recpt_rwait, recpt_rwait, NULL) != 0){	
		perror ("pthread_create");
		exit (1);
	}


	
	while (fin != 1){
		if (nb_serv == 0){
			printf("++++ Main : Plus de serveur distant disponible, deconnexion ++++ \n");
			kill (getpid (), SIGINT);
		}
	}

	printf ("++ Main : Fin ++ \n");
	close (sock_UDP);
	
	return EXIT_SUCCESS;
}
