#ifndef _RPROCTYPES_H
#define _RPROCTYPES_H 1

#include <bits/posix1_lim.h>
#include <sys/types.h>

/* taille max d'un nom de programme, ou d'une chaine */
#define PROG_NAME_MAX_LEN   64
#define STRLEN	64

/* taille max d'un nom de machine */
#define HOST_NAME_MAX_LEN _POSIX_HOST_NAME_MAX


/* Définition d'un type UID */
typedef struct {
    char host[HOST_NAME_MAX_LEN];
    pid_t pid;
} t_uid;

#endif
