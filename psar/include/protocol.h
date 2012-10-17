#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "rproctypes.h"
#include "fonctions.h"
#include <netinet/in.h>

/* port des serveurs */
#define PORTLOC  8800
#define PORTDIST 9900

/* interface serveur local pour le client */

#define PORT_REX  8810
#define PORT_RWAIT 8820
#define PORT_RKILL 8830
#define PORT_RPS 8850

#define NPROC_MAX           100

#define TIME_CLEAN 	1
#define SPEED_CLEAN 	5

/*********************
 * TYPES DE MESSAGES *
 *********************/

#define REXECUT		10
#define REXECUT_ACK	11
#define RWAIT		20
#define RWAIT_ACK	21
#define RKILL		30
#define RKILL_ACK	31
#define REXIT		41
#define RPS_QUERY	50
#define RPS_RESPONSE	51
#define FIN		91
#define RWAIT_FIN	71
#define TAB_CLEAN	86


#define ETAT_NULL         -1000
#define PROC_NULL         -1001
#define ETAT_EN_COURS     -2000
#define ETAT_STOPPE       -3000
#define ETAT_PAS_NE       -4000 
#define ETAT_TERMINE	  -5000

/*****************
 *  MESSAGES     *
 *****************/

typedef struct {
	int msg_type;
	char prog[PROG_NAME_MAX_LEN];
	char machine[HOST_NAME_MAX_LEN];
	pid_t emetteur;
	int id_req;
} t_rexecut;


typedef struct {
	int msg_type;
	t_uid uid;
	int id_req;
} t_rexecut_ack;


typedef struct {
	int msg_type;
	t_uid uid; /* pour un waituid */
	pid_t emetteur;
} t_rwait;

typedef struct {
        int msg_type;
        t_uid uid; /* pour un waituid */
        pid_t emetteur;
	struct sockaddr_in exp;
} t_rwait_int;


typedef struct {
	int msg_type;
	t_uid uid; 
	int status;
} t_rwait_ack;

typedef struct {
        int msg_type;
        t_uid uid;
} t_rwait_fin;


typedef struct {
	int msg_type;
	t_uid uid ;
	int signo;
	int id_req;
} t_rkill;


typedef struct {
	int msg_type;
	int val_ret ;
	int id_req;
} t_rkill_ack;


typedef struct {
	int msg_type;
	t_uid uid;
	int status;
} t_rexit;


typedef struct {
	int msg_type;
} t_rps_query;


typedef struct {
	int msg_type;
        t_uid uid;
        int status;	
} t_tab_clean;


typedef struct processus {
	int numero_processus;
	int etat_processus;
	int time_fin;
	int time_deb;
	int sock_pos;
	char prog[PROG_NAME_MAX_LEN];
} proc;

typedef struct processus_envoi {
	t_uid uid;
	int etat_processus;
	char prog[PROG_NAME_MAX_LEN];
	int time_fin;
        int time_deb;
}proce;


typedef struct {
	int msg_type;
	proce prog[NPROC_MAX];
} t_rps_response;

typedef struct {
	int msg_type;
	proce prog[NPROC_MAX*10];
} t_rps_response_client;



/* ------ Début ajout ------ */

typedef struct proces{ 
    t_uid uid;
    int status;
    int finished;
    int pid_emet;	
} t_proc;

/* ------ Fin ajout -------- */


#endif
