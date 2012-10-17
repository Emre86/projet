/**
 * Fichier: local.h
 * Description: en-tête pour le serveur local.
 **/

#include <netinet/in.h>

#include "protocol.h"


#define MAXREQ 16


typedef struct {
	t_rexecut query;
	struct sockaddr_in exp;
} rex_query;


typedef struct {
	t_rkill query;
	struct sockaddr_in exp;
} rkill_query;






