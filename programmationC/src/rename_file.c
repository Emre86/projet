#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include "srm.h"

/*
  fonction de renommage de fichier
  On renvoie le nouveau nom du fichier passe en parametre
 */

char* rename_file(char *file){
  int l;
  int ret=0;
  char *new_name;
  int taille_new_name;
  char c;
  int  tmp=0;
  int i=0;
  /*
    on initialise la graine avec la longueur du fichier le pid et un rand
   */

  l=strlen(file);
  srand(l+getpid()+rand());
  /*
   on fixe la nouvelle taille du nom du fichier avec rand modulo 
   125 + 2 pour avoir au moins un caractere
   */
  
  taille_new_name=(rand()%125)+2;
 
  /*
   Allocation du buffer contenant le nouveau nom
   */

  new_name=(char *)malloc(taille_new_name*sizeof(char));
  for(i=0;i<taille_new_name;i++){
    tmp=rand()%123;
    /*
     On recupere une valeur de rand modulo 123 car z=122 et
     on verfie si on a un caractere alphanumerique 
     */
    
    if((isalnum(tmp))!= 0)
      {
	c=(char) tmp;
	new_name[i]=c;

      }
    else{
      i--;
    }
  }
  /*
   on force la chaine a contenir un \0 pour indiquer la fin de chaine
   afin de ne pas recuperer un nom ne correspondant pas au critere souhaité
   */
  new_name[taille_new_name -1]='\0';
 
  /*
   On renomme le fichier
   */
  ret=rename(file,new_name);
  if(ret==-1){
    perror(">>>> error rename file\n");
  }
  /*
   On retourne le nouveau nom
   */
  return new_name;
}
