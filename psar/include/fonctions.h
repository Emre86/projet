/*
 * Fichier: fonctions.h
 * Description: diverses fonctions utilitaires
 *
 */
#ifndef _FONCTIONS_H
#define _FONCTIONS_H 1

#include "rproctypes.h"

/****************************************** 
 * uidcmp : permet de comparer 2 uid      *
 * Renvoie 0 si uid1 = uid2               *
 * c'est à dire si ils désignent le même  *
 * pid sur le même hôte                   *
 ******************************************/

int uidcmp(t_uid uid1, t_uid uid2);



/******************************************
 * strToVect : transforme la chaine de    *
 * caractères str en tableau de chaines *
 ******************************************/

void strToVect(char *str, char **args);

#endif
