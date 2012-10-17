#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include "srm.h"


/*
  Fonction parcourant les arborescenses et supprimant 
  les fichiers et les repertoires
 */
int remove_df(char *dir){
  int ret=0;
  DIR *dp;
  char *suprim;
  struct dirent *entry;
  struct stat statbuf;

  /* ouverture du repertoire
   */

  if ((dp = opendir(dir))== NULL){
    fprintf(stderr,">>> ne peut ouvrir ce repertoire %s \n",dir);
    return -1;
  }
  /* navigation dans le repertoire
   */

  chdir(dir);
  /*
   lecture du repertoire tant que le reperoire n est pas vide
   */
  while((entry = readdir(dp)) != NULL){
    /*
      Recuperation des informations 
     */
    lstat(entry->d_name,&statbuf);
    /*
     Verification si il s agit d un repertoire
     */
    if (S_ISDIR(statbuf.st_mode)){
      /*
       On ne s occupe pas de . et ..
       */
      if(strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0 )
	continue;
      /*
       Appel recursive a la fonction remove_df
       */
      remove_df(entry->d_name);
    }
    else{
      /*
       Suppression dans le cas ou on a un fichier et un lien symbolique
       */

      if (S_ISREG(statbuf.st_mode)){
	
	mult_write_file(entry->d_name);
	suprim=rename_file(entry->d_name);
	unlink(suprim);
      }
      if(S_ISLNK(statbuf.st_mode)){ 
	mult_write_file(entry->d_name);
	suprim=rename_file(entry->d_name);
	unlink(suprim);
      }
    } 
  }
  /*
    On revient au repertoire parent
   */
  chdir("..");
  /*
    On ferme le repertoire 
   */
  closedir(dp);
  /*
   On supprime le repertoire
   */
  rmdir(dir);
  return ret;
}
