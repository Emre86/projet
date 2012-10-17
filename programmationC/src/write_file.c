#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "srm.h"

/*
 Fonction de reecriture des donnes
 En parametre on a le nom du fichier l entier qui va servir 
 d initialisation de caractere et l entier boolean pour signifier si 
 on effectue une ecriture de donnees aleatoire
 */

int write_file(char *file,int ascii,int boolean){
  
  FILE *ecrit;
  FILE *random;  
  unsigned char c=(unsigned char)ascii;
  int incr=0;
  int lenw=0;
  int lenr=0;
  struct stat sts;
  int t_car=sizeof(char);
  int taille=0;
  int ret=0;

  /*
   Recuperation des informations sur le fichier
   */

  if ( stat (file ,&sts) != 0){
    fprintf(stderr,"%s : erreur %X\n",file,errno);
    return -1;
  }


  taille=(int)sts.st_size;
  /*
    Ouverture du fichier
   */

  ecrit=fopen(file,"w");
  if ( ecrit == NULL){
    fprintf(stderr,"Erreur ouverture du fichier %s",file);
    return -1;
  }
  /*
   Verification du boolean si ecriture de donnes aleatoires
   */
  if (boolean==1){
    
    /*
     Ouverture de /dev/urandom
     */
    random=fopen("/dev/urandom","r");
    if (random == NULL){
      fprintf(stderr,"Erreur ouverture du fichier /dev/urandom");
      return -1;
    }

    while(incr < taille ){
      /*
	lecture de /dev/urandom 
       */
      lenr=fread(&c,t_car,1,random);
      if ( t_car != lenr ){
	ret=-1;
	printf (">>> Error : read /dev/urandom\n");
	fclose(random);
	fclose(ecrit);
	return ret;
      }
      /*
       ecriture de la donnee lu
       */
      lenw = fwrite(&c,t_car,1,ecrit);
      if ( t_car != lenw ){
	ret=-1;
	printf (">>> Error : rewrite\n");
	fclose(ecrit);
	return ret;
      }
      
      incr++;
    }
    /*
      Fermeture de /dev/urandom
     */
    fclose(random);
  }
  /*
   Si le boolean est faux on ecrit la valeur passe en parametre
   */
  else{ 
    

    while(incr < taille ){
      /*
       Ecriture de la valeur passe en parametre
       */
      lenw = fwrite(&c,t_car,1,ecrit);
      if ( t_car != lenw ){
	ret=1;
	printf (">>> Error : rewrite\n");
	fclose(ecrit);
	return ret;
      }
      incr++;
    }
  }

  /*
   Fermeture du fichier ou l on a ecrit
   */
  fclose(ecrit);
 
  return ret;
}
