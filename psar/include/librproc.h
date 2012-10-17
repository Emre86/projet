/***
 * Fichier:	    librproc.h
 * Description :    interface pour l'utilisateur final
 ***/

#ifndef _LIBRPROC_H
#define _LIBRPROC_H 1

#include "rproctypes.h"

int rexecut(t_uid *uid, char *exec);
int rexec(t_uid *uid, char *exec, char *machine);
int rwait(int *status);
int rwaituid(t_uid uid, int *status);
int rkill(t_uid uid, int sig);


#endif
