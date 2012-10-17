#include "protocol.h"


/* Longueur maximale pour la file des connexions en attente 
 * (paramètre de l'appel 'listen')
 */
#define MAX_PENDING_CONNEXIONS 10


/* nombre maximum d'arguments */
#define NB_MAX_ARGS 32
#define ARG_LEN 64


/****** 
 * structure côté serveur des processus répartis :
 * char *nom_demandeur	: machine à l'origine du REXECUT
 * char *prog		: le nom de l'éxecutable
 * t_uid uid		: l'uid du processus réparti
 ******/

/* on pourra ajouter d'autres informations */

typedef struct {
    char *nom_demandeur;
    char *prog;
    t_uid uid;
} prog_entry;





 
