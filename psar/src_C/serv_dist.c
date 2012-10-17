/***
 **   Fichier  : serv_dist.c  **
 ***/

#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>

#include "fonctions.h"
#include "server.h"



extern int gethostname(char *nom, size_t lg);
int rexit(int status,int pid,int position);




/*************************************/
/********** Initialisations **********/
/*************************************/

/*** expliquer et mettre dans include ***/
#define NB_MAX_SERV 	10


/******* Initialisation pour les sockets *******/
struct sockaddr_in s_exp; /*Nom de l expediteur*/
struct hostent *hp;

/*** Socket pour la commmunication en UDP ***/
struct sockaddr_in udp;  /*Nom de la socket du serveur*/
int sock_UDP;

/*** Sockets pour la connexxion en TCP ***/
struct sockaddr_in tcp;  /*Nom de la socket du serveur pour rexecut*/
int sock_TCP;

/*** Sockets pour les echanges avec les clients en TCP***/
struct sockaddr_in ech[NB_MAX_SERV];
int sock_ech[NB_MAX_SERV];

/*** Socket temporaire pout TCP  ***/
struct sockaddr_in tmp;
int sock_tmp;


int fromlen = sizeof(struct sockaddr_in);

/******* Definition pour les threads *******/

/*Thread pour la reception des demandes rps*/
pthread_t tid_recept_rps;
pthread_attr_t attr_recept_rps;

/*Thread pour la reception des demandes rexecut*/
pthread_t tid_recept_rex;
pthread_attr_t attr_recept_rex;


/*Thread pour la reception*/
pthread_t tid_recp[NB_MAX_SERV];
pthread_attr_t attr_recp[NB_MAX_SERV];

/*Thread pour l'envoi*/
pthread_t tid_env[NB_MAX_SERV];
pthread_attr_t attr_env[NB_MAX_SERV];

/*Thread pour la reception rexecut*/
pthread_t tid_rex[NB_MAX_SERV];
pthread_attr_t attr_rex[NB_MAX_SERV];

/*Thread pour la reception rkill*/
pthread_t tid_rkill;
pthread_attr_t attr_rkill;

/*Thread pour le nettoyage*/
pthread_t tid_tab_clean;
pthread_attr_t attr_tab_clean;


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;



/******* Definition des signaux *******/
struct sigaction action;
sigset_t sig_set;

typedef struct tableau_serv{
	int presence;
}tableau_serv;
tableau_serv tab_serv[NB_MAX_SERV];

int nb_recu;
int nb_rex;


typedef struct rexec {
	int position;
	int num_rex;
	/*char work_dir[70];*/
	int id_req;
	char nom_exec[HOST_NAME_MAX_LEN];
}rexecut_req;

typedef struct {
	int position;
	t_uid uid;
	int signo;
	int id_req;
}rkill_req;





/****** Creation des variables pour rexecut *******/
int status ;  
pid_t p,p2;
int pid;


/******Creation tableau les processus sur la machine *******/

int aux = 0 ;
int pid_a_enlever = PROC_NULL ;

/* Varialbe pour stocker le pid du porgramme principal pour
 * ensuite le traitement des signaux */
int pid_pere;

proc processus_actif[NPROC_MAX];

/*************************************************/
/******************* FONCTIONS *******************/
/*************************************************/





void* tab_clean(){
	int time_courant;
	int i;
	int nb_clean = 0;
	t_tab_clean rep;

	while(1){
		 for(i=0;i<NPROC_MAX;i++){
			if ( processus_actif[i].time_fin > 0){
				time_courant = time(NULL);
				/*printf("------ tab_clean : time_courant = %d ------\n",time_courant);*/
				if ( ( time_courant - processus_actif[i].time_fin > (60 * TIME_CLEAN) ) && (  processus_actif[i].etat_processus == ETAT_TERMINE) ){
					printf("------ tab_clean : Nettoyage du processus %d ------ \n",processus_actif[i].numero_processus);


					rep.msg_type = TAB_CLEAN;
			                rep.uid.pid = processus_actif[i].numero_processus;
			                gethostname(rep.uid.host,HOST_NAME_MAX_LEN);
			                rep.status = 0;

					if ( tab_serv[processus_actif[i].sock_pos].presence == 1){
				                if ( write(sock_ech[processus_actif[i].sock_pos], &rep, sizeof(rep)) == -1) {
				                        perror("write");
                	        			exit(1);
				                }
					}					

					printf("------ tab_clean : Envoyer message au serv local en position %d \n",processus_actif[i].sock_pos);	

					processus_actif[i].etat_processus = ETAT_NULL;
        	                        processus_actif[i].numero_processus = PROC_NULL;
	                                strcpy(processus_actif[i].prog,"NULL");
                	                processus_actif[i].time_deb = -1 ;
                        	        processus_actif[i].time_fin = -1 ;
					nb_clean ++;
				}
			}
		}
		/*printf("------ tab_clean : Nettoyage effectué %d processus supprimes ------\n",nb_clean);*/
		/*sleep(SPEED_CLEAN);*/
		nb_clean = 0;
	}
	return NULL;
}








/***********************************************/
/********** Fonction pour l'execution **********/
/**********  des processus repartis   **********/
/***********************************************/



/*
 ** Fonction affiche_tableau
 ** Affiche les valeurs courantes du tableau des 
 ** processus lances
 **
 **/

void affiche_tableau(){
	int i=0;
	for(i=0;i<NPROC_MAX;i++){
		if( processus_actif[i].numero_processus != -1001 ){
			printf("----- affiche_tableau : processus_actif : etat=%d ",processus_actif[i].etat_processus);
			printf("numero=%d ",processus_actif[i].numero_processus);
			printf("nom programme = %s ",processus_actif[i].prog);
		}
	}
	printf("\n\n");
}


/*
 ** Fonction initialise_tableau
 ** Initialisation du tableau des processus actifs avec la
 ** valeur -1000 et -1001  au debut ( => aucun processus )
 **
 **/


void initialise_tableau(){
	int i=0;
	for(i=0;i<NPROC_MAX;i++){
		processus_actif[i].etat_processus=ETAT_NULL;
		processus_actif[i].numero_processus=PROC_NULL;
		strcpy(processus_actif[i].prog,"NULL");
		processus_actif[i].time_deb = -1 ;
		processus_actif[i].time_fin = -1 ;
        	processus_actif[i].sock_pos = -1 ;
	}
}

/*
 ** Fonction recherche_pid
 ** recherche un processus dans la structure des processus
 ** et renvoie sa posititon ou -1 si inexsitant
 **
 **/

int recherche_pid(int pid){
	int i=0;
	int flag=-1; 
	printf("------ recherche_pid : pid rechercher=%d ------ \n",pid);
	while(i< NPROC_MAX){ 
		if ((processus_actif[i].numero_processus)==pid){
			flag=1;
			break;
		}
		i++;
	}
	if (flag==-1){
		i=-1;
	}
	return i;
}


/*
 ** Fonction rkill
 ** va envoyer un signal au processus passe en parametre
 **
 **/

/* AJOUTER : SI sig == SIGCONT : remettre l'état valide */
int r_kill(pid_t pid ,int sig,int pos_id){
	int err;
	int position;
	if ((err=kill(pid,sig))==-1){
		fprintf(stderr,"erreur lors kill du processus %d\n",pid);
	}else{
		if ((sig == SIGINT ) ||  (sig == SIGKILL ) || (sig == SIGTERM )) {
			position = recherche_pid(pid);
			if (position != -1){
				/*processus_actif[position].etat_processus = ETAT_NULL ;
				processus_actif[position].numero_processus = PROC_NULL ;
		                strcpy(processus_actif[position].prog,"NULL");
				*/
				rexit(sig,pid,pos_id);

			}
		}else{  
			if ( (sig == SIGSTOP ) || (sig == SIGTSTP )){
				position = recherche_pid(pid);
				if (position != -1){
					processus_actif[position].etat_processus = ETAT_STOPPE ; 
				}
			}else{
				if ( sig == SIGCONT  ){
					position = recherche_pid(pid);
					if (position != -1){
						processus_actif[position].etat_processus = ETAT_EN_COURS; 
					}
				}
			}
		}
	}
	return err;
}



/*
 ** Fonction traiter_signal
 ** mise en place de routine pour les signaux
 **
 **/

void traiter_signal(int numero_signal) {
	int i=0;
	t_rexecut_ack mes_fin;
	/* si on est le processus principale */
	if ( getpid() == pid_pere ){ 
		printf("SIG+++ On m'a demandé d'arrêter le programme avec  un sigint.\n");
		close(sock_TCP);
		close(sock_UDP);
		for(i=0 ; i<nb_recu ; i++){
			if (tab_serv[i].presence == 1){
				mes_fin.msg_type = FIN;
		                printf("SIG+++ Envoie du message FIN %i  \n",i);
        		        /* Envoyer le message */
	                	if ( write(sock_ech[i],&mes_fin,sizeof(mes_fin)) == -1) {
	        	            perror("write");
        	        	    exit(1);
	        	        }
				close(sock_ech[i]);
			}
		}
	printf("SIG+++ Fermeture serveur distant\n");
	exit(1);
	}
}





/*
 ** Fonction rexit
 ** terminaison d'un processus en cours (pas fini)
 **
 **/

int rexit(int status,int pid,int position){

	t_rexit mes_rep;

	printf("------ rexit : pid => %d et status = %d ------ \n",pid,status);
	int pos = recherche_pid(pid);
	printf("------ rexit : position => %d et status = %d ------ \n",pos,status);
        processus_actif[pos].etat_processus = ETAT_TERMINE;
	processus_actif[pos].time_fin = time(NULL);

	/* envoyer un message rexit au serveur local */
	/**** recuper position ***/	

	if ( tab_serv[position].presence == 1){
	
		/* envoyer rexit */ 
		mes_rep.msg_type = REXIT;
		mes_rep.uid.pid = pid;
		gethostname(mes_rep.uid.host, HOST_NAME_MAX_LEN);
		mes_rep.status = status;
	
		if ( write(sock_ech[position], &mes_rep, sizeof(mes_rep)) == -1) {
	                perror("write");
        	        exit(1);
	        }
	}
	return status;	

}


/* 
 ** Fonction rexecut
 ** traite les demandes d'executions des requetes  du client
 **
 **/
    
int rexecut(void* arg){
	int incr;
	int cpt;
	/*** on doit recupérer la position trouvée auparavant ***/
        rexecut_req  req = *(rexecut_req*)arg;
	t_rexecut_ack mes_rep;

  	int status ;
	int tube[2];
	int tube2[2];
	int res;

	char executable[PROG_NAME_MAX_LEN];
	/* Tableau pour les des éventuels arguments */
	char *tab_args[NB_MAX_ARGS];
	

	pid_t pid_fils;

	printf("nom executable : %s\n",req.nom_exec);

	
	if  ((pipe(tube))==-1){
		perror("erreur tube\n");
		/*exit(EXIT_FAILURE);*/
		kill(SIGINT,getppid());
	}

	if  ((pipe(tube2))==-1){
                perror("erreur tube");
                /*exit(EXIT_FAILURE);*/
		kill(SIGINT,getppid());
        }

	strToVect(req.nom_exec, (char**)&tab_args);
	strcpy(executable, tab_args[0]);


	if ((p=fork())==0){

		if ((p2=fork())==0){

			if( (execvp(executable, tab_args))==-1){
				exit(-1);		
			}
			
		}else{

		close (tube[0]);
		write(tube[1],&p2,sizeof(pid_t));
		wait(&status);
		close(tube2[0]);
		write(tube2[1],&status,sizeof(int));
		

		printf("------ rexecut: fin du fils n°1 et status = %d  ------\n",status);
		printf("rechercher_pid => %d\n",p2);
		exit (EXIT_SUCCESS) ;
		}
	}else{

		close (tube[1]);
        	read(tube[0],&pid_fils,sizeof(pid_t));

		incr=0;
                while(incr<(2*NPROC_MAX)){
                        if(processus_actif[cpt].numero_processus ==  PROC_NULL ){
                                processus_actif[cpt].numero_processus  = pid_fils;
                                processus_actif[cpt].sock_pos = req.position;
                                processus_actif[cpt].etat_processus    = ETAT_EN_COURS ;
		                strcpy(processus_actif[cpt].prog,req.nom_exec);
				processus_actif[cpt].time_deb = time(NULL);
				processus_actif[cpt].time_fin = - 1;
                                printf("------ rexecut : dans le while pid_fils = %d , nom exec = %s et etat = %d ------ \n",pid_fils,processus_actif[cpt].prog,processus_actif[cpt].etat_processus);
                                affiche_tableau();
                                printf("position_while %d \n",cpt);
				cpt++;
                                break;
                        }else{
                                cpt++;
                                incr++;
                        }
                        cpt=cpt%NPROC_MAX;
                }

		/* envoyer rexecut_ack */ 
		mes_rep.msg_type = REXECUT_ACK;
		mes_rep.id_req = req.id_req;
		gethostname(mes_rep.uid.host, HOST_NAME_MAX_LEN);
		mes_rep.uid.pid = pid_fils;
	
		if ( write(sock_ech[req.position],&mes_rep,sizeof(mes_rep)) == -1) {
                	perror("write");
	                exit(1);
        	}
		printf("---- rexecut_ack envoyé sur position %d, type %d, id : %d\n", req.position, mes_rep.msg_type, mes_rep.id_req);

		close (tube2[1]);
        	read(tube2[0],&res,sizeof(int));
		rexit(res,pid_fils,req.position);
		printf("rexecut fin pere %d \n",res);
	}
	return pid_fils ;
} 




/** 
 ** Fonction rkill
 ** traite les demandes d'executions des requetes RKILL  du client
 **
 **/
    
void rkill(void* arg){

	t_rkill_ack mes_rep;
	int val_ret;

	/*** on doit recupérer la position trouvée auparavant ***/
	rkill_req  req = *(rkill_req*)arg;


	val_ret = r_kill(req.uid.pid, req.signo,req.position);

	/* envoyer rkill_ack */ 
	mes_rep.msg_type = RKILL_ACK;
	mes_rep.id_req = req.id_req;
	mes_rep.val_ret = val_ret;


	if ( write(sock_ech[req.position],&mes_rep,sizeof(t_rkill_ack)) == -1) {
        	perror("write");
            exit(1);
	}

	printf("---- rkill_ack envoyé sur position %d, type %d, id : %d, val %d\n", req.position, mes_rep.msg_type, mes_rep.id_req, mes_rep.val_ret);
		
}

/*************************************************************/
/********** Partie Communication avec serveur local **********/
/*************************************************************/

/*
 ** Fonction recevoir_mess
 ** pour recevoir des messages
 ** des clients
 **
 **/

void* recevoir_mess(void* arg){

	t_rexecut req_rex;
	rexecut_req tmp_rex;
	rexecut_req *pt_rex;

	t_rkill req_rkill ;
	rkill_req tmp_rkill;
	rkill_req *pt_rkill;

	t_rwait_fin req_rwait;

	int type;
	int nb_lu;
	int position;

	/*** on doit recupere la position trouver auparavant ***/
	int pos = *(int*)arg; 

	/*** Attente d'un message ***/
	while(1){
		printf("Lecture type\n");
	  
		if ( (nb_lu = recv(sock_ech[pos],&type, sizeof(int), MSG_PEEK|MSG_WAITALL)) == -1) {
			perror("recv ");
		}else { 

			/*printf ("NB_LU %d\n", nb_lu);*/
		

			switch(type) {
			case REXECUT :

				if ( read(sock_ech[pos], &req_rex, sizeof(t_rexecut)) == -1) {
					perror("read");
				 exit(1);
				}
	
				printf("---- recevoir_mess : message de type = %d ; prog = %s ---- \n", req_rex.msg_type, req_rex.prog);
	
				nb_rex = (nb_rex + 1) % NB_MAX_SERV ;


				/*** Creation threads pour l'execution du rexecut ***/
				if( pthread_attr_init(&attr_rex[nb_rex]) != 0){
					perror("pthread_attr_init");
			  		 exit(1);
			  	}
				tmp_rex.position = pos;
				tmp_rex.num_rex = nb_rex;

				tmp_rex.id_req = req_rex.id_req;
				strcpy(tmp_rex.nom_exec, req_rex.prog);
			
				/*printf("ARGV[0] : %s\n", req_rex.args[0]);*/
	
				pt_rex = (rexecut_req *)malloc(sizeof(tmp_rex));	
				*pt_rex = tmp_rex;

				if( pthread_create(&tid_rex[nb_rex],&attr_rex[nb_rex],(void*)rexecut,pt_rex) != 0){
					perror("pthread_create");
			  		exit(1);
				}
				break;

			case FIN :
				read(sock_ech[pos], &type, sizeof(int)); /* pour consommer le type dans la file !! */
				printf("---- recevoir_mess : Fin du serveur local en position %d ---- \n",pos);
				nb_recu --;
				close(sock_ech[pos]);
				if ( pthread_cancel( pthread_self() ) != 0 ){ 
					perror("pthread_cancel"); 
				}
				tab_serv[pos].presence = 0;
				break;

			case RKILL :
				if ( read(sock_ech[pos], &req_rkill, sizeof(t_rkill)) == -1) {
					perror("read");
					exit(1);
				}
	

				printf("---- recevoir_mess : message de type = %d ; pid = %d signal %d\n", req_rkill.msg_type, req_rkill.uid.pid, req_rkill.signo);
	
				tmp_rkill.position = pos;
				printf("%d = %d ?\n", tmp_rkill.position, pos);
				tmp_rkill.uid = req_rkill.uid;
				tmp_rkill.signo = req_rkill.signo;
				tmp_rkill.id_req = req_rkill.id_req;

	
				pt_rkill = (rkill_req *)malloc(sizeof(tmp_rkill));	
				*pt_rkill = tmp_rkill;
	
				/*** Creation threads pour l'execution du rkill ***/
				if( pthread_attr_init(&attr_rkill) != 0){
					perror("pthread_attr_init");
			 		exit(1);
				}

				if( pthread_create(&tid_rkill, &attr_rkill, (void*)rkill, pt_rkill) != 0){
					perror("pthread_create");
			 		exit(1);
				}

				break;
			case RWAIT_FIN:
				 if ( read(sock_ech[pos], &req_rwait, sizeof(t_rwait_fin)) == -1) {
                                        perror("read");
                                        exit(1);
                                }
				position = recherche_pid(req_rwait.uid.pid);
				printf("------ recevoir_mess : position => %d  ------ \n",position);
				processus_actif[position].etat_processus = ETAT_NULL;
				processus_actif[position].numero_processus = PROC_NULL;
				strcpy(processus_actif[position].prog,"NULL");
				processus_actif[position].time_deb = -1 ;
				processus_actif[position].time_fin = -1 ;
				processus_actif[position].sock_pos = -1 ;

				printf("---- recevoir_mess : message de type = %d ; pid = %d \n", req_rwait.msg_type, req_rwait.uid.pid);
				


				break;

			default:
				printf("Type inconnu %d\n", type);
				break;
	
	    		}
			
		}




        } /* fin while(1) */
        return NULL;
}




void* Traitement_RPS(){

        t_rps_response rep;
	int i;
        rep.msg_type = RPS_RESPONSE;
	for ( i=0 ; i< NPROC_MAX ; i++){
		rep.prog[i].uid.pid = processus_actif[i].numero_processus;
		rep.prog[i].etat_processus = processus_actif[i].etat_processus;
		strcpy(rep.prog[i].prog,processus_actif[i].prog);
		gethostname(rep.prog[i].uid.host, HOST_NAME_MAX_LEN);
		rep.prog[i].time_deb = processus_actif[i].time_deb;
		rep.prog[i].time_fin = processus_actif[i].time_fin;
	}
        
	printf("------ Traiement_RPS : Traitement_Arrivee_Client en cours ------ \n");
        if (sendto(sock_UDP,&rep,sizeof(rep),0,(struct sockaddr *)&s_exp,fromlen) == -1) {
                perror("sendto erreur");
                exit(1);
        }
        printf("------ Traitement_RPS : Traitement_Arrivee_Client termine ------ \n");
        return NULL;
}



/*
 **  Fonction qui traite la reception la connexion des clients
 **  sur le serveur en UDP
 **
 **/

void* recept_RPS(void* arg){

        t_rps_query req;
        t_rps_response rep;

        while(1){
                if ( recvfrom(sock_UDP,&req, sizeof(req), 0, (struct sockaddr *) &s_exp,(socklen_t*) &fromlen) == -1) {
                        perror("recvfrom");
                	kill(SIGINT,getppid());
		        exit(1);
                }
		printf("---- recept_RPS : Reception d'une demande RPS ---\n");
	
                /*** Recuperation du nom de la machine ***/
                if (( hp = gethostbyname((char *) &s_exp.sin_addr.s_addr) ) == NULL){
                	if ((hp = gethostbyaddr((char *)&s_exp.sin_addr.s_addr,sizeof(s_exp.sin_addr),AF_INET)) == NULL) {
			        perror("gethostbyaddr");
				kill(SIGINT,getppid());
                        	exit(1);
			}
                }

                /*** Affichage des donnes sur le message ***/
                printf("---- recept_RPS : Message Recu Expediteur : <IP = %s, PORT = %d, MACHINE = %s, TYPE = %d> ---- \n", inet_ntoa(s_exp.sin_addr), ntohs(s_exp.sin_port),hp->h_name,req.msg_type);

                /*** Traitement des messages ***/
                if ( req.msg_type == RPS_QUERY ){
                        Traitement_RPS();
                }else{
                        rep.msg_type = -1;
                        if (sendto(sock_UDP,&rep,sizeof(rep),0,(struct sockaddr *)&s_exp,fromlen) == -1) {
                                perror("sendto");
				kill(SIGINT,getppid());
                                exit(1);
                        }
                }

        }
        return NULL;
}


/*
 **  Fonction qui traite la reception la connexion des clients
 **  sur le serveur en TCP
 **
 **/

void* recept_REX(void* arg){
	
	nb_recu = 0;
	int *pt;	
	int opt = 1;	

	if ((sock_TCP = socket(AF_INET,SOCK_STREAM,0)) < 0) {
                perror("creation socket");
		kill(SIGINT,getppid());
                exit(1);
        }


        /* remplir le nom */
        bzero((char *)&tcp,sizeof(tcp));
        tcp.sin_addr.s_addr = htonl(INADDR_ANY);
        tcp.sin_port = htons(PORTDIST);
        tcp.sin_family = AF_INET;

	if (setsockopt (sock_TCP, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (int)) == -1){
		perror ("setsockopt");
		exit (1);
	}



	/* nommage */
        if (bind(sock_TCP,(struct sockaddr *)&tcp,sizeof(tcp)) < 0) {
                perror("bind");
		kill(SIGINT,getppid());
                exit(2);
        }

	if ( listen(sock_TCP,10) == -1){
		perror("listen");
		kill(SIGINT,getppid());
	}
	
	printf("---- recept_REX : On ecoute sur la socket ---- \n");

	/*** on se met en attente d'un client ***/
	for(;;){
		
		if( (sock_tmp = accept(sock_TCP,(struct sockaddr *)&tmp,(socklen_t*)&fromlen)) == -1){
			printf("erreur accept\n");
			kill(SIGINT,getppid());
			exit(3);
		}
		printf("---- recept_REX : Accept reussi client n° %d ---- \n",nb_recu);
		sock_ech[nb_recu] = sock_tmp;
		
		/*** Creation threads pour communication avec ce client ***/
		if( pthread_attr_init(&attr_recp[nb_recu]) != 0){
                	perror("pthread_attr_init");
			kill(SIGINT,getppid());
        	        exit(1);
	        }

		pt = (int*)malloc(sizeof(nb_recu));
		*pt = nb_recu;

		if( pthread_create(&tid_recp[nb_recu],&attr_recp[nb_recu],recevoir_mess,pt) != 0){
               		perror("pthread_create");
			kill(SIGINT,getppid());
                	exit(1);
        	}
		printf("---- recept_REX : Creation d'un thread pour reception rexecut ---- \n");
		
		tab_serv[nb_recu].presence = 1;
		nb_recu ++;

	}


	return NULL;
}



int main(int argc, char **argv){


/*************************************/
/***** Initialisation du serveur *****/
/*************************************/
	int opt = 1;
	sigset_t sig_set ; 
	struct sigaction action ; 
	nb_rex = 0;
	
	pid_pere = getpid();
	sigemptyset(&sig_set); 
	action.sa_mask = sig_set ; 
	action.sa_flags = 0 ; 
	action.sa_handler = traiter_signal ; 
 
	/*atexit((void *)traiter_signal);*/

	sigaction(SIGINT  , &action , NULL); 
	sigaction(SIGTERM , &action , NULL); 

	/***Initialisation du tableau ***/
	initialise_tableau(); 

	/* creation de la socket */
        if ((sock_UDP = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
                perror("creation socket");
		kill(SIGINT,getppid());
                exit(1);
        }

        printf("-- Main : Creation de la socket reussi -- \n");
	
	/* remplir le nom */
        bzero((char *)&udp,sizeof(udp));
        udp.sin_addr.s_addr = htonl(INADDR_ANY);
        udp.sin_port = htons(PORTDIST);
        udp.sin_family = AF_INET;


	if (setsockopt (sock_UDP, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (int)) == -1){
		perror ("setsockopt");
		exit (1);
	}


        /* nommage */
        if (bind(sock_UDP,(struct sockaddr *)&udp,sizeof(udp)) < 0) {
                perror("bind");
		kill(SIGINT,getppid());
                exit(2);
        }

        printf("-- Main : Bind sur la socket reussi et lancement du thread de reception --\n");

        /*** Threads de connexion ***/
        if ( pthread_attr_init(&attr_recept_rps) !=0 ){
                perror("pthread_attr_init");
		kill(SIGINT,getppid());
                exit(1);
        }
        if ( pthread_create(&tid_recept_rps,&attr_recept_rps,recept_RPS,NULL) != 0){
                perror("pthread_create");
		kill(SIGINT,getppid());
                exit(1);
        }
	
	if ( pthread_attr_init(&attr_recept_rex) !=0 ){
                perror("pthread_attr_init");
		kill(SIGINT,getppid());
                exit(1);
        }
        if ( pthread_create(&tid_recept_rex,&attr_recept_rex,recept_REX,NULL) != 0){
                perror("pthread_create");
		kill(SIGINT,getppid());
                exit(1);
        }

	if ( pthread_attr_init(&attr_tab_clean) !=0 ){
                perror("pthread_attr_init");
                kill(SIGINT,getppid());
                exit(1);
        }
        if ( pthread_create(&tid_tab_clean,&attr_tab_clean,tab_clean,NULL) != 0){
                perror("pthread_create");
                kill(SIGINT,getppid());
                exit(1);
        }

	

        while(1){
                sigsuspend(&sig_set);
        }


	return EXIT_SUCCESS;




}

