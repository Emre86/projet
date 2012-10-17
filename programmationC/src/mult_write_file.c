#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "srm.h"

/*
  Cette fonction fait 6 appel a la fonction write_file
*/


int mult_write_file (char *file){
  int ret=0;
  int flag=0;
  struct stat sts;


  /*
   Recuperation des informations associes au fichier
   */
  if ( stat (file ,&sts) != 0){
    fprintf(stderr,"%s : erreur %X\n",file,errno);
    return -1;
  }
  /*
    On verifie si l on a les droits en ecriture sur le fichier et 
    on positionne le flag a 1 pour signifier que l on peut faire 
    la  reecriture dans le fichier
   */
  if (sts.st_mode & S_IWUSR){ 
    flag=1;
  }
  if (sts.st_mode & S_IWGRP){
    flag=1;
  }
  if (sts.st_mode & S_IWOTH){
    flag=1;
  }
  if (flag == 0){
    return -1;
  }

  /*
   Remplie de zero
   */

  ret=write_file(file,0,0);
  if(ret==-1){
    return ret;
  }
  /*
   Remplie de ff
   */

  ret=write_file(file,255,0);
  if(ret==-1){
    return ret;
  }
  /*
   Remplie d aleatoire
   */  
  
  ret=write_file(file,0,1);
  if(ret==-1){
    return ret;
  }
  /*
   Remplie de ff
   */
  ret=write_file(file,255,0);
  if(ret==-1){
    return ret;
  }
  /*
   Remplie de zero
   */
  ret=write_file(file,0,0);
  if(ret==-1){
    return ret;
  }
  /*
   Remplie d aleatoire
   */
  ret=write_file(file,0,1);
  if(ret==-1){
    return ret;
  }
  
  return ret;

}
