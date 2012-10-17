/** Client **/

#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#include "protocol.h"

/************************************/
/********** Initalisations **********/
/************************************/

extern int gethostname(char *nom, size_t lg);




/************************************/
/********** Fonctions ***************/
/************************************/


int minute(int s){
  return ( s / 60) % 60;
}

int seconde(int s){
	return s%60;

}

int heure(int s){
  return (s/3600 + 2)%24;
}

int main(int argc, char *argv[])
{
	int i;
	int rps_query;
	int rps_resp;
	int trouve = 0;
	

	t_rps_query query;
	t_rps_response_client resp;
	int cas = 0;
        char name[15];
	int nb_rps = 0;


	char tmp[HOST_NAME_MAX_LEN + 6] = "cliloc";
        char tmp2[HOST_NAME_MAX_LEN + 6] = "loccli";
        char mpt[HOST_NAME_MAX_LEN];
        char mpt2[HOST_NAME_MAX_LEN];

        gethostname(mpt,HOST_NAME_MAX_LEN);
        strcat(tmp,mpt);

        gethostname(mpt2,HOST_NAME_MAX_LEN);
        strcat(tmp2,mpt2);


	if ( argc == 2){
		if( strcmp(argv[1],"-a") == 0){
			cas = 1;
		}
	}	

	/*** Ouverture du tube pour envoyer vers le serveur local ***/
	if ( ( rps_query = open(tmp,O_WRONLY)) == -1){
	 	perror("open");
		exit(1);
	}
	
	query.msg_type = RPS_QUERY;
	/*** Envoie du message dans le tube ***/	
	if( (write(rps_query,&query,sizeof(t_rps_query))) == -1){
		perror("write");
		exit(1);
	}
	
	close(rps_query);
		
	/*** Ouverture du tube pour recevoir des messages venant du serveur local ***/
	if(( rps_resp = open(tmp2,O_RDONLY)) == -1){
                perror("open");
                exit(1);
        }


	/*** Reception du message dans le tube ***/
	if( read(rps_resp,&resp,sizeof(t_rps_response_client)) == -1){
		perror("read");
		exit(1);
	}	
	
	for ( i = 0 ; i < NPROC_MAX;i++){
		if( resp.prog[i].uid.pid > 0){
			
		        if ( resp.prog[i].etat_processus == -1000 ){
        	        	strcpy(name,"ETAT_NULL");
        		}
		        if ( resp.prog[i].etat_processus == -1001 ){
        		        strcpy(name,"PROC_NULL");
		        }
        		if ( resp.prog[i].etat_processus == -2000 ){
                		strcpy(name,"ETAT_EN_COURS");
		        }
	        	if ( resp.prog[i].etat_processus == -3000 ){
		                strcpy(name,"ETAT_STOPPE");
	        	}
		        if ( resp.prog[i].etat_processus == -4000 ){
                		strcpy(name,"ETAT_PAS_NE");
		        }	
		        if ( resp.prog[i].etat_processus == -5000 ){
                		strcpy(name,"ETAT_TERMINE");
		        }	
			if ( cas == 0){	
				if ( resp.prog[i].etat_processus != -5000){
					printf(" UID : < %s , %d >  ||  ETAT : %s || NOM : %s \n",resp.prog[i].uid.host,resp.prog[i].uid.pid,name,resp.prog[i].prog);
					trouve = 1;
					strcpy(name,"");
					nb_rps ++;
				}
			}else{
				
				if ( resp.prog[i].etat_processus == -5000){
					nb_rps++;	
					printf(" UID : < %s , %d >  ||  ETAT : %s || NOM : %s || TIME_DEB : <%d,%d,%d> || TIME_FIN : <%d,%d,%d> \n",resp.prog[i].uid.host,resp.prog[i].uid.pid,name,resp.prog[i].prog,heure(resp.prog[i].time_deb),minute(resp.prog[i].time_deb),seconde(resp.prog[i].time_deb),heure(resp.prog[i].time_fin),minute(resp.prog[i].time_fin),seconde(resp.prog[i].time_fin));
				}else{
					nb_rps++;

					printf(" UID : < %s , %d >  ||  ETAT : %s || NOM : %s || TIME_DEB : <%d,%d,%d> \n",resp.prog[i].uid.host,resp.prog[i].uid.pid,name,resp.prog[i].prog,heure(resp.prog[i].time_deb),minute(resp.prog[i].time_deb),seconde(resp.prog[i].time_deb));
				}
				trouve = 1;
				strcpy(name,"");
			}		
		}
	}
	if ( trouve == 0 ){
		printf(" Pas de processus en cours \n");
	}
	printf(" RPS %d Processus\n",nb_rps);
	close(rps_resp);
 	

	

        return(0);
}

