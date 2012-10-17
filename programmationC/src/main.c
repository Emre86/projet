#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "srm.h"

int main (int argc,char **argv){
  /* 
     Declaration de la variable nom pour supprimer le fichier apres le 
     changement de son nom
   */
  char *nom;
 
  /*
   Deux arguments pour l'aide 
   */

  if ((argc == 2)){
    
    if (strcmp(argv[1],"-h")==0){
      printf("Suppression d un fichier ./srm -f nom_du_fichier\n");
      printf("Suppression d un repertoire ./srm -d nom_du_repertoire\n");
      return 0;
    }
    else{
      fprintf(stderr,"Erreur argument\n./srm -h\n");
      return -1;
    }
  }

  /*
   Trois arguments le second pour l'option -f ou -d
   et le troisieme poue le nom de fichier ou de repertoire
   Si -f on supprime le fichieres fichiers,sous-repertoires etc et le repertoire
   Si -d on supprime tout 
   */
  if (argc == 3){
    if (strcmp(argv[1],"-f")==0){
      mult_write_file(argv[2]);
      nom=rename_file(argv[2]);
      unlink(nom);
      return 0;
    }
  
    if (strcmp(argv[1],"-d")==0){
      remove_df(argv[2]);
    }
    else{
      fprintf(stderr,"Erreur argument\n./srm -h\n");
      return -1;
    }
  }
  else{
    fprintf(stderr,"Erreur nombre d argument\n./srm -h\n");
    return -1;
  }

  return 0;
  
}
